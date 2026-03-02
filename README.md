[![Apache licensed](https://img.shields.io/badge/License-Apache--2.0-cyan?logo=apache)](https://github.com/lynx-family/animax/blob/main/LICENSE)

A high-performance animation library for cross-platform applications. It is fully compatible with the Lottie format, and uniquely supports standalone playback of Alpha Videos as well as seamless mixing of Lottie 2D vector layers and Alpha Video layers.

## Content
- [About AnimaX](#About-AnimaX)
- [AnimaX Export Plugin (AFX)](#AnimaX-Export-Plugin-AFX)
- [Documentation](#Documentation)
- [How to Use AnimaX](#How-to-Use-AnimaX)
- [How to Contribute](#How-to-Contribute)
- [Discussions](#Discussions)
- [Credits](#Credits)
- [License](#License)

## Showcase

Here are some stunning animations rendered seamlessly by AnimaX:

| <img src="https://lf-lynx.tiktok-cdns.com/obj/lynx-artifacts-oss-sg/animax/showcase/video_demo_1.gif" width="250"/> | <img src="https://lf-lynx.tiktok-cdns.com/obj/lynx-artifacts-oss-sg/animax/showcase/mixed_image_video_demo.gif" width="250"/> | <img src="https://lf-lynx.tiktok-cdns.com/obj/lynx-artifacts-oss-sg/animax/showcase/vector_demo_2.gif" width="250"/> |
| :----------------------------------------------------------- | :----------------------------------------------------------- | :----------------------------------------------------------- |
| Pure Alpha Video Layer                                       | Mixed With Text And Alpha Video Layer                        | Vector Layer                                                 |
| <img src="https://lf-lynx.tiktok-cdns.com/obj/lynx-artifacts-oss-sg/animax/showcase/vector_demo_3.gif" width="250"/> | <img src="https://lf-lynx.tiktok-cdns.com/obj/lynx-artifacts-oss-sg/animax/showcase/text_demo.gif" width="250"/> | <img src="https://lf-lynx.tiktok-cdns.com/obj/lynx-artifacts-oss-sg/animax/showcase/image_demo_3.gif" width="250"/> |
| Vector Layer With Effects                                    | Text Layer                                                   | Image Layer                                                  |

## Why AnimaX?

While Lottie popularized vector animations on mobile and web, reproducing complex After Effects features often forces compromises. Designers may have to rely on large image sequences which bloat app sizes, face inconsistent rendering across iOS and Android due to differing platform implementations, or suffer performance drops because traditional Lottie players run heavy parsing and rendering on the main thread.

AnimaX bridges this gap by maintaining full compatibility with the standard After Effects and Lottie workflow while completely re-architecting the runtime engine:

- **Full Lottie Compatibility & Zero-Cost Migration**: AnimaX seamlessly parses and plays existing Lottie JSON files. You can drop AnimaX into your project as a direct replacement for traditional Lottie players with zero code changes required.
- **Uncompromised Performance**: Unlike traditional Lottie players that compete for main thread resources, AnimaX offloads the most time-consuming graphics rendering entirely to background threads, ensuring silky-smooth frame rates even on low-end devices.
- **Consistent Cross-Platform Rendering**: Powered by a unified C++ rendering engine [Skity](https://github.com/lynx-family/skity), AnimaX ensures pixel-perfect consistency across platforms without relying on fragmented native APIs.
- **Breakthrough Engine Capabilities**: Overcome Lottie's inability to render complex AE effects. AnimaX uniquely supports mixing standard 2D vector layers with Alpha Video (transparent video) layers, unlocking limitless visual potential without sacrificing file size or performance.
- **Advanced Interactivity**: Built-in support for dynamic properties allows you to modify animation attributes at runtime or respond to user interactions seamlessly.

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
