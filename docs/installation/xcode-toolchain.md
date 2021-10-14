# Xcode toolchain

1. Download the toolchain.
2. Copy or symlink the toolchain into `~/Library/Developer/Toolchains/`.
3. Select Spoor from Xcode's toolchains list: `Xcode > Toolchains > Spoor`.

```bash
mkdir -p ~/Library/Developer/Toolchains
```

```bash
ln -sfn /Users/you/path/to/downloaded/MicrosoftSpoor.xctoolchain /Users/you/Library/Developer/Toolchains/MicrosoftSpoor.xctoolchain
```

![Xcode toolchain selection](xcode-toolchain-selection.png)
