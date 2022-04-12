# Reference

_**spoor**_<br />
noun | the track or scent of an animal<br />
verb | follow or scent of (an animal or person)

!!! info "Metaphor"
    Your application is the animal and the function traces are the scent it
    leaves behind.

## Introduction

Spoor gives you deep insight into your application's performance. Its three-part
toolchain enables you to analyze your application down to the function call with
nanosecond precision and includes:

1. [Compiler instrumentation][instrumentation] to auto-inject trace events.
2. A [runtime library][runtime] to capture and buffer events.
3. [Tools][postprocessing] to process and visualize the traces.

## Goals

Spoor is designed to achieve the following goals.

* Develop a tool capable of collecting an _exhaustive_ application trace
  including function entries and exists with highly accurate timestamps.
* Be language- and platform-independent.
* Have a virtually imperceptible performance impact when turned off and a
  "usable" performance impact when turned on. This includes deferring as much
  processing as possible to an offline postprocessing tool.
* Provide configurable parameters that allow developers to tune the runtime
  performance (speed, memory, and disk space) of the tracing, possibly at the
  expense of dropping events.
* Allow the instrumented application to dynamically enable tracing, disable
  tracing, and flush the buffers.
* Achieve all runtime features in-process.

## Project status

Microsoft's Outlook team uses Spoor internally to identify logical and
performance problems on its iOS client.

However, the project is still in its infancy and should be considered
alpha-quality software. Please anticipate breaking changes,
[report bugs][provide-feedback] that you encounter, and consider
[contributing code][contribute-code] to support Spoor's development.

## Alternatives

### XRay

[XRay][xray] is a toolchain that also collects high-accuracy function traces
with the ability to dynamically enable and disable tracing at runtime. At a high
level, XRay's architecture is similar to Spoor's because it combines
compiler-inserted instrumentation with a runtime library to collect traces.

However, XRay's implementation and underlying architecture differs. XRay
dynamically enables and disables tracing by patching the binary at runtime to
insert or remove function entry and exit logs whereas Spoor checks if the
runtime is enabled before logging.

Although XRay's approach is more efficient, is does not work on Apple platforms
because the kernel does not permit patching the binary at runtime due to the
resulting security and performance implications.

XRay is open sourced as part of the LLVM project.

[https://llvm.org/docs/XRay.html][xray]

### dtrace

[dtrace][dtrace] is a static and dynamic tracing framework that samples an
application's runtime stack.

Unlike Spoor, dtrace's dynamic tracing can capture non-instrumented code
including operating system kernel calls and precompiled libraries, however, does
not run in-process and captures samples (i.e., not an exhaustive trace) of an
application.

[https://en.wikipedia.org/wiki/DTrace][dtrace]

[contribute-code]: contributing/#contribute-code
[dtrace]: https://en.wikipedia.org/wiki/DTrace
[instrumentation]: instrumentation
[postprocessing]: postprocessing
[provide-feedback]: contributing/#provide-feedback
[runtime]: runtime
[xray]: https://llvm.org/docs/XRay.html
