#pragma once
#include <string>

// Small shared text helpers. Header-only so no CMake target changes.
//
// Both live here because three call sites used to cut strings with a blind
// substr(0, N): the ReAct observation stored into LLM history, the window
// titles in Executor's list_windows, and the ReAct fallback summary. A blind
// cut lands mid-word ("--clean-f", "Spotif"), which is audible when the result
// is spoken. tts.cpp:303 already had the right idea (rfind(' ', N)); this
// lifts that one pattern so every caller shares it.
namespace textutil {

// Drop ANSI/VT escape sequences and carriage returns. Terminal tools like
// neofetch emit ~1.6 bytes per token of pure escape noise, which is both
// unspeakable and (once it lands in LLM history) ruinously expensive to
// re-prefill on every subsequent turn.
inline std::string stripAnsi(const std::string& in) {
    std::string out;
    out.reserve(in.size());
    for (size_t i = 0; i < in.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(in[i]);
        if (c == 0x1b) {                       // ESC
            if (i + 1 < in.size() && (in[i + 1] == '[' || in[i + 1] == ']')) {
                char introducer = in[i + 1];
                i += 2;
                if (introducer == '[') {       // CSI: params then a final letter
                    while (i < in.size() &&
                           !((in[i] >= 'A' && in[i] <= 'Z') ||
                             (in[i] >= 'a' && in[i] <= 'z')))
                        ++i;
                } else {                       // OSC: runs to BEL or ST
                    while (i < in.size() && in[i] != '\a' &&
                           !(in[i] == 0x1b && i + 1 < in.size() && in[i + 1] == '\\'))
                        ++i;
                    if (i < in.size() && in[i] == 0x1b) ++i;
                }
            } else {
                ++i;                           // two-char escape, e.g. ESC(B
            }
            continue;
        }
        if (c == '\r') continue;
        out += in[i];
    }
    return out;
}

// Truncate to at most n characters, backing up to the last space so the cut
// never lands mid-word. Falls back to a hard cut only when no space sits in
// the back half of the range.
inline std::string truncateAtWord(const std::string& in, size_t n,
                                  const std::string& ellipsis = "...") {
    if (in.size() <= n) return in;
    size_t cut = in.rfind(' ', n);
    if (cut == std::string::npos || cut < n / 2) cut = n;
    return in.substr(0, cut) + ellipsis;
}

}  // namespace textutil
