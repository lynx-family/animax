[![Apache licensed](https://img.shields.io/badge/License-Apache--2.0-cyan?logo=apache)](https://github.com/lynx-family/animax/blob/main/LICENSE)

A high-performance Lottie animation library for cross-platform applications.

## Content
- [About AnimaX](#About-AnimaX)
- [AnimaX Export Plugin (AFX)](#AnimaX-Export-Plugin-AFX)
- [Documentation](#Documentation)
- [How to Use AnimaX](#How-to-Use-AnimaX)
- [How to Contribute](#How-to-Contribute)
- [Discussions](#Discussions)
- [Credits](#Credits)
- [License](#License)

## About AnimaX

AnimaX is an open-source, high-performance Lottie animation rendering library designed to deliver smooth and efficient animations across mobile platforms.
- **Lottie Animation Support.** Render Adobe After Effects animations exported as JSON with full fidelity and flexibility.
- **Cross-Platform.** Native support for Android and iOS with consistent rendering behavior.
- **High Performance.** Optimized rendering engine with GPU acceleration and efficient resource management.
- **Dynamic Properties.** Modify animation properties at runtime for interactive and personalized experiences.
- **Alpha Video Support.** Built-in support for video elements within animations.



## AnimaX Export Plugin (AFX)

AFX is an Adobe After Effects plugin that allows designers to export their designs to Lottie-compatible formats. If there are layers that need to be exported as alpha video layers, you must use AFX to export them. The exported artifacts can be played seamlessly by AnimaX.

For more details, please refer to the [AFX Plugin Documentation](./export/README.md).

## Documentation
You shall find documentation for AnimaX on [lynxjs.org](https://lynxjs.org/next/guide/animax/introduction.html).

## How to Use AnimaX
### Requirements
AnimaX apps may target iOS 10 and Android 5.0 (API 21) or newer.

We recommend using macOS as the development operating system. Windows and Linux are not yet verified or guaranteed, so you may encounter problems. If you need assistance, please file an issue, and we will be more than happy to help you address it.

### Getting Started guide
Getting started guide [here](https://lynxjs.org/next/guide/animax/start/quick_start.html)

## How to Contribute
### [Code of Conduct][coc]
We are devoted to ensuring a positive, inclusive, and safe environment for all contributors. Please find our [Code of Conduct][coc] for detailed information.

[coc]: CODE_OF_CONDUCT.md

### [Contributing Guide][contributing]
We welcome you to join and become a member of AnimaX Authors. It's people like you that make this project great.

Please refer to our [contributing guide][contributing] for details.

[contributing]: CONTRIBUTING.md

## Discussions
Bugs and feature requests are filed in [Github Issues](https://github.com/lynx-family/animax/issues)

Large discussions and proposals are discussed in [Github Discussions](https://github.com/lynx-family/animax/discussions)

## Credits
AnimaX makes use of several third-party libraries and draws inspiration from various projects. We would like to express our sincere gratitude to these sources.

### Third Party Libraries
AnimaX incorporates the following third-party libraries, which have significantly contributed to its functionality. We appreciate the efforts of the developers and the open-source community behind these projects:
- [skity](https://github.com/lynx-family/skity)
- [lynx-textra](https://github.com/lynx-family/lynx-textra)
- [lynx-base](https://github.com/lynx-family/lynx/blob/develop/base/README.md)
- [rapidjson](https://github.com/Tencent/rapidjson)

### Referenced API Design and Implementations
The design of AnimaX has been inspired by and referenced from the following outstanding projects. Their innovative designs and solutions have been invaluable in shaping AnimaX:
- [lottie-android](https://github.com/airbnb/lottie-android)
- [lottie-web](https://github.com/airbnb/lottie-web)

We respect the intellectual property rights of all these projects and adhere to relevant open-source licenses and usage guidelines.

## [License][license]
AnimaX is Apache licensed, as found in the [LICENSE][license] file.

[license]: LICENSE
