# Spoor [![Build Status][build-status-badge]][build-status]

![Wikipedia iOS app boot trace][wikipedia-trace-png]

Spoor gives you deep insight into your application's performance. Its three-part
toolchain enables you to analyze your application down to the function call with
nanosecond precision and includes:

1. [Compiler instrumentation][site-instrumentation] to auto-inject trace events.
2. A [runtime library][site-runtime] to capture and buffer events.
3. [Tools][site-postprocessing] to process and visualize the traces.

--------------------------------------------------------------------------------

### Browse the documentation and tutorials on Spoor's website.

### [www.spoor.dev][site]

[![www.spoor.dev screenshot][spoor-website-screenshot]][site]

--------------------------------------------------------------------------------

## Contributor License Agreement

Most contributions require you to agree to a Contributor License Agreement (CLA)
declaring that you have the right to, and actually do, grant us the rights to
use your contribution. For details, visit
[cla.opensource.microsoft.com][microsoft-cla].

When you submit a pull request, the CLA bot will automatically determine whether
you need to provide a CLA and decorate the PR appropriately (e.g., status check,
comment). Simply follow the instructions provided by the bot. You will only need
to do this once across all repos using our CLA.

## Code of Conduct

This project has adopted the
[Microsoft Open Source Code of Conduct][code-of-conduct]. For more information
see the [Code of Conduct FAQ][code-of-conduct-faq] or contact
[opencode@microsoft.com][opencode-email] with any additional questions or
comments.

## Trademarks

This project may contain trademarks or logos for projects, products, or
services. Authorized use of Microsoft trademarks or logos is subject to and must
follow [Microsoft’s Trademark & Brand Guidelines][trademark-brand-guidelines].
Use of Microsoft trademarks or logos in modified versions of this project must
not cause confusion or imply Microsoft sponsorship. Any use of third-party
trademarks or logos are subject to those third-party’s policies.

[build-status-badge]: https://github.com/microsoft/spoor/actions/workflows/build-and-test.yml/badge.svg?branch=master
[build-status]: https://github.com/microsoft/spoor/actions/workflows/build-and-test.yml
[code-of-conduct-faq]: https://opensource.microsoft.com/codeofconduct/faq/
[code-of-conduct]: https://opensource.microsoft.com/codeofconduct/
[microsoft-cla]: https://cla.opensource.microsoft.com
[opencode-email]: mailto:opencode@microsoft.com
[site-instrumentation]: https://www.spoor.dev/reference/instrumentation
[site-postprocessing]: https://www.spoor.dev/reference/postprocessing
[site-runtime]: https://www.spoor.dev/reference/runtime
[site]: https://www.spoor.dev
[spoor-website-screenshot]: docs/spoor-website-screenshot.png
[trademark-brand-guidelines]: https://www.microsoft.com/en-us/legal/intellectualproperty/trademarks
[wikipedia-trace-png]: docs/wikipedia-ios-boot-trace.png
