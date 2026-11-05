// ==WindhawkMod==
// @id              opera-active-tab-return-native
// @name            Opera Active Tab Return (native tab handler)
// @description     Click the active tab to toggle between the top and your reading position. Opera 139.0.6053.22 and .38 x64.
// @version         1.3.2
// @author          bbmaster123/Astra
// @include         opera.exe
// @architecture    x86-64
// @compilerOptions -lshell32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
Experimental!!! Please back up your entire opera profile just in case something happens!
Restores behavior of "Scroll to the top of the page by clicking the active tab" as it was in Opera 131 and lower. 
This allows users to click the active tab to scroll to the top of the webpage, and click again to return to their previous position. 
In versions 132+ they have decided to remove the second click action entirely, which is the motivation for this mod.

Currently only supports v139.0.6053.22 and v139.0.6053.38 and will need to be updated as new opera builds are released. 
Not tried, but I would imagine this could also work on Opera GX (and the other one too, the chill one whatever that's named) without much modification. 

known limitations:
- requires update for each opera update. If using an unsupported build, this mod stops without changing anything.
- Opera must be installed in its default location (for now)
- does not (yet) work on many internal pages such as settings, extensions, history, flags, etc
- accel setting not yet working
- scroll animation is not exactly 100% identical to the original built in feature, though it is close enough IMO

*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- durationMs: 280
  $name: Scroll duration (ms)
  $description: Both directions. Use 1–2000 ms for custom timing, or 0 for the original Opera smooth animation.
- rampPercent: 15
  $name: Acceleration and deceleration (% each)
  $description: Each end takes this percentage of the animation time. 0 is linear, 15 gives short ramps, and 50 gives continuous easing.
*/
// ==/WindhawkModSettings==

#include <string>
#include <windows.h>
#include <shellapi.h>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cwchar>

namespace {
using Bytes = unsigned char;

// Per-build RVAs are matched from disassembly and checked against the actual
// loaded vtables. CodeView includes the linker GUID and PDB age.
struct Build {
    const wchar_t* version;
    DWORD timestamp;
    DWORD imageSize;
    Bytes codeView[24];
    size_t tabHandler;
    size_t scrollToTop;
    size_t contentsTable;
    size_t frameTable;
    size_t getFocusedFrame;
    size_t getMainFrame;
    size_t frameLive;
    size_t frameActive;
    size_t executeIsolated;
};

constexpr Build builds[] = {
    // DLL SHA-256 f2c379b31a88a2757b98f06497357b5a08cf84ff0807ed4e9ca5b39731f49d72
    {L"139.0.6053.22", 0x6ae22e3e, 0x1113b000,
        {0x52, 0x53, 0x44, 0x53, 0x1b, 0xda, 0x05, 0x95, 0xd5, 0xde, 0x3e, 0x56,
         0x4c, 0x4c, 0x44, 0x20, 0x50, 0x44, 0x42, 0x2e, 0x01, 0x00, 0x00, 0x00},
        0x31a8590, 0x879a630, 0xea32c70, 0xe942640, 0x2727ab0,
        0x54513f0, 0x5bc88a0, 0x6010440, 0x62aa5d0},
    // DLL SHA-256 cb3c79b05135299433680ee0048567b157a7b7acc42b06d23c61f16dbdf4d477
    {L"139.0.6053.38", 0x6ae8a121, 0x11146000,
        {0x52, 0x53, 0x44, 0x53, 0xbc, 0x5a, 0x89, 0xd2, 0x16, 0xa7, 0x49, 0xf0,
         0x4c, 0x4c, 0x44, 0x20, 0x50, 0x44, 0x42, 0x2e, 0x01, 0x00, 0x00, 0x00},
        0x31a9610, 0x879fef0, 0xea391e0, 0xe948aa0, 0x2727b80,
        0x5456ca0, 0x5bce150, 0x6015d00, 0x62afe90},
};
const Build* build;
Bytes* image;
using TabHandler = void (*)(void*, void*);
using ScrollToTop = void (*)(void*);
TabHandler originalTab;
ScrollToTop originalTop;
thread_local unsigned tabDispatchDepth;
std::atomic_bool ready{false};
HANDLE stopEvent;
HANDLE installer;

constexpr char16_t script[] = uR"SCROLL(
((duration, rampPercent) => {
  const key = Symbol.for('windhawk.opera-active-tab-return.timed.1');
  const root = document.scrollingElement;
  if (!root) return;

  // The stored state belongs to this document in the browser's isolated world.
  // An input that starts a new reading position ends the previous toggle.
  let state = globalThis[key];
  if (state && (!state.target.isConnected || state.url !== location.href)) {
    state.cancel();
    state = null;
  }

  function scheduleScroll() {
    // Two clicks before the next paint can cancel each other completely.
    // Starting then reversing a smooth scroll in one compositor frame can
    // otherwise leave Blink a couple of pixels away from the saved position.
    cancelAnimationFrame(state.frame);
    state.frame = requestAnimationFrame(startTime => {
      const target = state.target;
      const scroller = target === root ? window : target;
      const top = state.atTop ? 0 : state.readingY;
      if (!state.started && !state.atTop) {
        state.cancel();
        return;
      }
      state.started = true;
      state.appliedTop = state.atTop;
      state.timed = duration > 0;
      if (state.timed) {
        const from = target.scrollTop;
        const ramp = rampPercent / 100;
        // Integrate a velocity profile with short linear acceleration ramps.
        // The middle section runs at constant speed; total area equals 1.
        function step(now) {
          const t = Math.min(1, (now - startTime) / duration);
          let distance;
          if (t < ramp) distance = t * t / (2 * ramp);
          else if (t > 1 - ramp) distance = 1 - ramp - (1 - t) ** 2 / (2 * ramp);
          else distance = t - ramp / 2;
          const y = t === 1 ? top : from + (top - from) * distance / (1 - ramp);
          scroller.scrollTo({left: target.scrollLeft, top: y, behavior: 'instant'});
          if (t < 1 && from !== top) state.frame = requestAnimationFrame(step);
          else if (!state.atTop) state.cancel();
        }
        step(startTime);
        return;
      }
      scroller.scrollTo({left: target.scrollLeft, top, behavior: 'smooth'});
      if (Math.abs(target.scrollTop - top) <= 4) {
        // An earlier animation may already be in flight on the compositor.
        state.frame = requestAnimationFrame(() => {
          scroller.scrollTo({left: target.scrollLeft, top, behavior: 'instant'});
          if (!state.atTop) state.cancel();
        });
      }
    });
  }

  if (state) {
    state.atTop = !state.atTop;
    scheduleScroll();
    return;
  }

  function scrolledAncestor(node) {
    while (node && node !== root) {
      if (node.scrollTop > 0 &&
          /^(auto|scroll|overlay)$/.test(getComputedStyle(node).overflowY)) return node;
      node = node.parentElement || node.getRootNode().host;
    }
    return null;
  }

  const target = scrolledAncestor(document.activeElement) ||
    (root.scrollTop > 0 ? root : scrolledAncestor(
      document.elementFromPoint(innerWidth / 2, innerHeight / 2)));
  if (!target) return;

  const controller = new AbortController();
  const listen = {capture: true, passive: true, signal: controller.signal};
  function cancel() {
    cancelAnimationFrame(state.frame);
    controller.abort();
    delete globalThis[key];
  }
  state = {target, readingY: target.scrollTop, url: location.href, atTop: true, started: false, frame: 0, cancel};
  globalThis[key] = state;
  for (const event of ['wheel', 'touchstart', 'pointerdown', 'pagehide', 'popstate', 'hashchange']) {
    window.addEventListener(event, cancel, listen);
  }
  window.addEventListener('keydown', e => {
    if (['ArrowUp', 'ArrowDown', 'PageUp', 'PageDown', 'Home', 'End', ' '].includes(e.key)) cancel();
  }, listen);
  const scrollEvents = target === root ? document : target;
  scrollEvents.addEventListener('scrollend', e => {
    if (state.timed || e.target !== scrollEvents) return;
    if (!state.atTop && state.appliedTop === false && Math.abs(target.scrollTop - state.readingY) < 1) cancel();
  }, listen);
  scheduleScroll();
})(
)SCROLL";

const Build* Recognize(Bytes* base) {
    const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
    const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS64*>(base + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE || nt->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64)
        return nullptr;

    const auto dir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];
    const auto* entries = reinterpret_cast<const IMAGE_DEBUG_DIRECTORY*>(base + dir.VirtualAddress);
    for (const auto& candidate : builds) {
        if (candidate.timestamp != nt->FileHeader.TimeDateStamp ||
            candidate.imageSize != nt->OptionalHeader.SizeOfImage) continue;
        for (size_t i = 0; i < dir.Size / sizeof(*entries); ++i) {
            const auto& entry = entries[i];
            if (entry.Type == IMAGE_DEBUG_TYPE_CODEVIEW && entry.AddressOfRawData &&
                entry.SizeOfData >= sizeof(candidate.codeView) &&
                std::memcmp(base + entry.AddressOfRawData, candidate.codeView, sizeof(candidate.codeView)) == 0)
                return &candidate;
        }
    }
    Wh_Log(L"OperaReturn: unsupported DLL fingerprint; timestamp=%08lX imageSize=%08lX",
           nt->FileHeader.TimeDateStamp, nt->OptionalHeader.SizeOfImage);
    return nullptr;
}

template<typename T>
T Address(size_t rva) { return reinterpret_cast<T>(image + rva); }

void OnTabDispatch(void* observer, void* tab) {
    ++tabDispatchDepth;
    originalTab(observer, tab);
    --tabDispatchDepth;
}

void OnScrollToTop(void* contents) {
    if (!tabDispatchDepth || !ready.load(std::memory_order_acquire) ||
        *static_cast<void**>(contents) != image + build->contentsTable) {
        originalTop(contents);
        return;
    }

    using Getter = void* (*)(void*);
    using Predicate = bool (*)(void*);
    void* frame = Address<Getter>(build->getFocusedFrame)(contents);
    if (!frame) frame = Address<Getter>(build->getMainFrame)(contents);
    if (!frame || *static_cast<void**>(frame) != image + build->frameTable ||
        !Address<Predicate>(build->frameActive)(frame) || !Address<Predicate>(build->frameLive)(frame)) {
        Wh_Log(L"OperaReturn: no active live frame; using Opera's scroll-to-top.");
        originalTop(contents);
        return;
    }

    // Read-only long-string view for this build's libc++ alternate ABI.
    // The serializer reads the high tag byte, length, and UTF-16 data and
    // copies the bytes into a Mojo message before the call returns.
    struct OperaString {
        const char16_t* data;
        size_t length;
        uint64_t taggedCapacity;
    };
    int duration = Wh_GetIntSetting(L"durationMs");
    int ramp = Wh_GetIntSetting(L"rampPercent");
    if (duration < 0 || duration > 2000) duration = 280;
    if (ramp < 0 || ramp > 50) ramp = 15;
    const std::string args = std::to_string(duration) + "," + std::to_string(ramp) + ");";
    std::u16string command(script);
    command.append(args.begin(), args.end());
    const size_t count = command.size() + 1;
    static_assert(sizeof(OperaString) == 24);
    const OperaString source{command.data(), count - 1, (uint64_t{1} << 63) | count};
    void* nullCallback = nullptr;
    using Execute = void (*)(void*, const OperaString*, void**, int);
    // 139 accepts browser-owned isolated world IDs 1 through 11.
    Address<Execute>(build->executeIsolated)(frame, &source, &nullCallback, 11);
    Wh_Log(L"OperaReturn: accepted tab click; sent Blink toggle.");
}

bool Install(HMODULE module) {
    image = reinterpret_cast<Bytes*>(module);
    build = Recognize(image);
    if (!build) {
        wchar_t path[MAX_PATH]{};
        GetModuleFileNameW(module, path, MAX_PATH - 1);
        Wh_Log(L"OperaReturn: skipped %ls", path);
        return false;
    }

    // Confirm both object layouts and the target of the native tab-handler's
    // tail call before registering either hook.
    auto* contents = Address<void**>(build->contentsTable);
    auto* frame = Address<void**>(build->frameTable);
    if (contents[0x450 / 8] != image + build->scrollToTop ||
        contents[0x98 / 8] != image + build->getFocusedFrame ||
        contents[0x80 / 8] != image + build->getMainFrame ||
        frame[0x248 / 8] != image + build->frameActive ||
        frame[0x230 / 8] != image + build->frameLive ||
        frame[0x1c0 / 8] != image + build->executeIsolated) {
        Wh_Log(L"OperaReturn: private interface mismatch; skipped.");
        return false;
    }
    if (!Wh_SetFunctionHook(image + build->tabHandler, reinterpret_cast<void*>(OnTabDispatch),
                            reinterpret_cast<void**>(&originalTab)) ||
        !Wh_SetFunctionHook(image + build->scrollToTop, reinterpret_cast<void*>(OnScrollToTop),
                            reinterpret_cast<void**>(&originalTop))) {
        Wh_Log(L"OperaReturn: hook registration failed.");
        return false;
    }
    return true;
}

DWORD WINAPI InstallWhenLoaded(void*) {
    do {
        // Take a module reference for the installation. Opera owns the DLL for
        // the browser lifetime, and never lends us a frame across a message.
        HMODULE module = nullptr;
        if (GetModuleHandleExW(0, L"opera_browser.dll", &module)) {
            if (Install(module)) {
                if (Wh_ApplyHookOperations()) {
                    ready.store(true, std::memory_order_release);
                    Wh_Log(L"OperaReturn: native tab hook ready (%ls x64).", build->version);
                } else {
                    Wh_Log(L"OperaReturn: Windhawk could not apply the hooks.");
                }
            }
            FreeLibrary(module);
            break;
        }
    } while (WaitForSingleObject(stopEvent, 40) == WAIT_TIMEOUT);
    return 0;
}
}  // namespace

BOOL Wh_ModInit() {
    int count = 0;
    auto argv = CommandLineToArgvW(GetCommandLineW(), &count);
    if (!argv) return FALSE;
    bool browser = true;
    for (int i = 1; i < count; ++i) {
        if (std::wcsncmp(argv[i], L"--type=", 7) == 0 || std::wcscmp(argv[i], L"--type") == 0) {
            browser = false;
            break;
        }
    }
    LocalFree(argv);
    if (!browser) return FALSE;
    stopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    return stopEvent != nullptr;
}

void Wh_ModAfterInit() {
    installer = CreateThread(nullptr, 0, InstallWhenLoaded, nullptr, 0, nullptr);
    if (!installer) Wh_Log(L"OperaReturn: install thread failed: %lu", GetLastError());
}

void Wh_ModBeforeUninit() {
    SetEvent(stopEvent);
    if (installer) WaitForSingleObject(installer, INFINITE);
    ready.store(false, std::memory_order_release);
}

void Wh_ModUninit() {
    if (installer) CloseHandle(installer);
    if (stopEvent) CloseHandle(stopEvent);
}

// Settings are read at the next accepted tab click.
void Wh_ModSettingsChanged() {}
