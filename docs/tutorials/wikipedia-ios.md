# Wikipedia iOS (Xcode toolchain)

This tutorial uses Spoor to instrument
[Wikipedia's iOS app][wikipedia-ios-github].

You'll learn Spoor's pipeline to automatically inject instrumentation into your
source code, build and run your Spoor-instrumented program, symbolize the trace,
and visualize and analyze your symbolized trace using Perfetto.

## 0. Prerequisites

### 0.1 Setup and dependencies

* `MicrosoftSpoor.xctoolchain`: Spoor's Xcode toolchain
* `spoor`: Postprocessing tool
* Xcode

!!! info "Version"
    This tutorial uses the following tools. Your mileage may vary with newer
    or older versions.

    **MicrosoftSpoor.xctoolchain**

    ```
    0.0.0
    ```

    **Xcode**

    ```bash
    xcodebuild -version
    ```

    ```
    Xcode 13.0
    Build version 13A233
    ```

    **spoor**

    ```bash
    spoor --version
    ```

    ```
    spoor 0.0.0
    ```

### 0.2 Download the code

=== "Git (SSH)"
    ```bash
    git clone \
      --branch releases/6.8.1 \
      --depth 1 \
      git@github.com:wikimedia/wikipedia-ios.git
    ```
=== "Git (HTTPS)"
    ```bash
    git clone \
      --branch releases/6.8.1 \
      --depth 1 \
      https://github.com/wikimedia/wikipedia-ios.git
    ```
=== "cURL"
    ```bash
    curl \
      --location \
      --output wikipedia-ios.tar.gz \
      https://github.com/wikimedia/wikipedia-ios/archive/refs/tags/releases/6.8.1.tar.gz &&
    tar -xf wikipedia-ios.tar.gz &&
    mv wikipedia-ios-releases-6.8.1 wikipedia-ios
    ```

```bash
cd wikipedia-ios
```

!!! info "Version"
    This tutorial is based on Wikipedia iOS version 6.8.1. Your mileage may vary
    with newer or older versions.

!!! check "Sanity test"
    Build and run the (uninstrumented) app to make sure that everything works
    as expected.

    :material-timer-sand: A clean build takes around four minutes (depending on
    how baller your computer is).

    === "UI"
        ```bash
        open Wikipedia.xcodeproj
        ```
        
        Build and run: `Product > Run (⌘R)`.

        ![Product > Run][xcode-product-run]

    === "Command line"
        ```bash
        xcodebuild build \
          -project Wikipedia.xcodeproj \
          -scheme Wikipedia \
          -configuration Debug \
          -sdk iphonesimulator \
          -arch x86_64
        ```

        ```bash
        open -a Simulator.app
        ```

        ```bash
        xcrun simctl \
          install \
          booted \
          ~/Library/Developer/Xcode/DerivedData/Wikipedia-xxxxxxxxxxxxxxxxxxxxxxxxxxxx/Build/Products/Debug-iphonesimulator/Wikipedia.app
        ```

        ```bash
        xcrun simctl launch booted org.wikimedia.wikipedia
        ```
    The app launches in the iOS Simulator.

    ![Wikipedia app start screen][wikipedia-start-screen]

## 1. Instrument the app

=== "UI"
    Select Spoor from Xcode's toolchains list: `Xcode > Toolchains > Spoor`.

    ![Xcode > Toolchains > Spoor][xcode-xcode-toolchains-spoor]

    Build: `Product > Build (⌘B)`.

    ![Product > Build][xcode-product-build]
=== "Command line"
    ```bash
    xcodebuild build \
      -project Wikipedia.xcodeproj \
      -scheme Wikipedia \
      -configuration Debug \
      -sdk iphonesimulator \
      -arch x86_64 \
      -toolchain Spoor
    ```

Spoor generates a `.spoor_symbols` file next to each object file.

```bash
find ~/Library/Developer/Xcode/DerivedData/Wikipedia-xxxxxxxxxxxxxxxxxxxxxxxxxxxx \
  -name "*.spoor_symbols"
```

```
/Users/you/Library/Developer/Xcode/DerivedData/Wikipedia-xxxxxxxxxxxxxxxxxxxxxxxxxxxx/Build/Intermediates.noindex/Wikipedia.build/Debug-iphonesimulator/WMF.build/Objects-normal/x86_64/NavigationState.spoor_symbols
/Users/you/Library/Developer/Xcode/DerivedData/Wikipedia-xxxxxxxxxxxxxxxxxxxxxxxxxxxx/Build/Intermediates.noindex/Wikipedia.build/Debug-iphonesimulator/WMF.build/Objects-normal/x86_64/NSDate+WMFRelativeDate.spoor_symbols
/Users/you/Library/Developer/Xcode/DerivedData/Wikipedia-xxxxxxxxxxxxxxxxxxxxxxxxxxxx/Build/Intermediates.noindex/Wikipedia.build/Debug-iphonesimulator/WMF.build/Objects-normal/x86_64/CIContext+WMFImageProcessing.spoor_symbols
/Users/you/Library/Developer/Xcode/DerivedData/Wikipedia-xxxxxxxxxxxxxxxxxxxxxxxxxxxx/Build/Intermediates.noindex/Wikipedia.build/Debug-iphonesimulator/WMF.build/Objects-normal/x86_64/NSError+WMFExtensions.spoor_symbols
/Users/you/Library/Developer/Xcode/DerivedData/Wikipedia-xxxxxxxxxxxxxxxxxxxxxxxxxxxx/Build/Intermediates.noindex/Wikipedia.build/Debug-iphonesimulator/WMF.build/Objects-normal/x86_64/CacheGroup+CoreDataProperties.spoor_symbols
...
```

## 2. Run with instrumentation

Create a directory to hold the trace files and
[configure Spoor's runtime][runtime-configuration] to save the traces in that
path.

```bash
mkdir trace
```

=== "UI"
    Configure Wikipedia's scheme with a `SPOOR_RUNTIME_TRACE_FILE_PATH`
    _run action_ environment variable: `Wikipedia > Edit Scheme…`.

    **Environment variables**

    Name                          | Value
    ----------------------------- | ---------------
    SPOOR_RUNTIME_TRACE_FILE_PATH | /path/to/trace/

    ![Wikipedia edit scheme][wikipedia-edit-scheme]

    Run: `Product > Run (⌘R)`.

    ![Product > Run][xcode-product-run]
    
    !!! warning "Appling environment variables"
        Environment variables are only applied when launching the app from Xcode
        (i.e., not when tapping the app's icon on the homescreen).
=== "Command line"
    Install the instrumented build.

    ```bash
    xcrun simctl \
      install \
      booted \
      ~/Library/Developer/Xcode/DerivedData/Wikipedia-xxxxxxxxxxxxxxxxxxxxxxxxxxxx/Build/Products/Debug-iphonesimulator/Wikipedia.app
    ```

    Launch the app with a `SPOOR_RUNTIME_TRACE_FILE_PATH` environment variable
    configuration.

    ```bash
    SIMCTL_CHILD_SPOOR_RUNTIME_TRACE_FILE_PATH="/path/to/trace/" xcrun simctl \
      launch \
      booted \
      org.wikimedia.wikipedia
    ```

Spoor's runtime emits trace files in the `trace` folder.

```bash
ls trace
```

```
xxxxxxxxxxxxxxxx-xxxxxxxxxxxxxxxx-xxxxxxxxxxxxxxxx.spoor_trace
...
```

## 3. Process and analyze the trace

Finally, parse and symbolize the trace data to view it in
[Perfetto's trace viewer][perfetto-viewer].

### 3.1 Parse and symbolize the trace

Use `spoor`, Spoor's postprocessing tool, to parse the `.spoor_trace` files in
your `trace` folder, symbolize the trace with the `spoor_symbols` files in
`DerivedData`, and output a Perfetto-compatible trace.

```bash
spoor \
  /path/to/trace/ \
  ~/Library/Developer/Xcode/DerivedData/Wikipedia-xxxxxxxxxxxxxxxxxxxxxxxxxxxx \
  --output_file=wikipedia-ios.perfetto
```

### 3.2 Visualize the trace

Open `wikipedia-ios.perfetto` in [ui.perfetto.dev][perfetto-ui], Perfetto's
trace viewer.

The flame graph timeline visualization shows the stack trace of the app's boot
over time.

![Wikipedia iOS app boot][wikipedia-boot-visualization]

[perfetto-ui]: https://ui.perfetto.dev/
[perfetto-viewer]: https://perfetto.dev/#viewer
[runtime-configuration]: /configuration/runtime/#spoor_runtime_trace_file_path
[wikipedia-boot-visualization]: wikipedia-boot-visualization.png
[wikipedia-edit-scheme]: wikipedia-edit-scheme.png
[wikipedia-ios-github]: https://github.com/wikimedia/wikipedia-ios
[wikipedia-start-screen]: wikipedia-start-screen.png
[xcode-product-build]: xcode-product-build.png
[xcode-product-clean-build-folder]: xcode-product-clean-build-folder.png
[xcode-product-run]: xcode-product-run.png
[xcode-xcode-toolchains-spoor]: xcode-xcode-toolchains-spoor.png
