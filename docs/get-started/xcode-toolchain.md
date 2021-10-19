# Xcode Toolchain

1. [Download][github-releases] the toolchain.
2. Copy or symlink the toolchain into `~/Library/Developer/Toolchains/`.
   ```bash
   mkdir -p ~/Library/Developer/Toolchains
   ```
   ```bash
   ln -sfn path/to/downloaded/MicrosoftSpoor.xctoolchain ~/Library/Developer/Toolchains/MicrosoftSpoor.xctoolchain
   ```
3. Select Spoor from Xcode's toolchains list: `Xcode > Toolchains > Spoor`.
   ![Xcode toolchain selection][xcode-toolchain-selection]

[github-releases]: https://github.com/microsoft/spoor/releases
[xcode-toolchain-selection]: xcode-toolchain-selection.png
