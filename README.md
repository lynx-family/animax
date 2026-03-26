# AnimaX - Cross-platform Animation Framework

AnimaX is a high-performance, cross-platform animation engine written in C++. This repository contains both open source and proprietary components.

## Project Overview

AnimaX provides a powerful animation framework for building complex animations across multiple platforms. The project follows a hybrid development model where core components are open source, while platform-specific implementations remain proprietary.

## Project Structure

This project contains the complete AnimaX project with both open source and proprietary components:

```
.
├── animax/          # Core animation engine library (Open Source)
├── example/         # Usage examples and demos (Proprietary)
├── testing/         # Unit tests and integration tests (Proprietary)
├── tools/           # Build tools and utilities (Proprietary)
├── platform/        # Platform-specific implementations (Proprietary)
├── src/             # Integration code for proprietary components
├── CPPLINT.cfg      # C++ code style configuration
├── LICENSE          # Apache 2.0 License
├── OWNERS           # Project maintainers
└── README.md        # Project documentation
```

### Open Source Components

The `animax/` directory contains the core animation engine library that is synchronized to the public GitHub repository `lynx-family/animax`:

```
animax/
├── example/         # Build AnimaXExample for Android/iOS without Lynx engine
├── include/         # Public header files of AnimaX
├── platform/        # Platform-specific implementations (Open source part)
├── src/             # Source code of AnimaX (Open source part)
├── third_party/     # Third-party libraries for open source builds
└── tools/           # Build tools for open source components
```

### Proprietary Components

The following directories contain proprietary code that is not open sourced:
- **platform/**: Platform-specific implementations (ByteVC1, image processing, monitoring)
- **src/**: Integration code for ByteVC1 and other proprietary technologies
- **example/**: Usage examples and demonstration projects
- **testing/**: Unit tests and integration tests
- **tools/**: Build tools and development utilities

## Open Source Strategy

Based on the discussion about [AnimaX Open Source Repository Transformation](https://bytedance.larkoffice.com/wiki/EddowJDoIitLK1kzwvzcW0VdnEh), AnimaX maintains a hybrid approach with both open source and proprietary components. The open source code is synchronized to the `lynx-family/animax` repository, while proprietary components are kept private for the following reasons:

1. **Second-party Dependencies**: Some code relies on second-party libraries that are only used internally at ByteDance, making the dependent code unsuitable for open source release.

2. **Testing Infrastructure Limitations**: Certain testing CI tasks cannot be deployed to GitHub yet. For example, the conformance test suite depends on macOS machines and the OSMesa project, both of which are not currently available in the open source environment.

## Development Workflow

Unlike the Skity project, AnimaX has a different development workflow. We cannot develop directly on GitHub and sync changes back to the internal repository because the proprietary components also require frequent updates and iterations.

Following a similar approach to the Lynx project, we maintain the entire project in the `template-assembler/animax` directory, with the open source components located in the `template-assembler/animax/animax` subdirectory.

The `animax` subdirectory is intentionally not named "oss" or "open-source" because it will be deployed to GitHub workflows. We prefer to avoid exposing the open source nature through directory names to external developers.

## Example Projects

The example directory contains comprehensive demonstrations of AnimaX functionality. It includes the `lottie_adapter_example`, which uses the LottieAdapter to compare animations between AnimaX and Lottie.

**Note**: The LottieAdapter itself is not open source as it relies on Lottie libraries that are only used within the ByteDance group.

## Platform-specific Implementations

The platform directory contains platform-specific code implementations, including:
- ByteVC1 video codec integration
- Image processing components
- Monitoring and performance tracking

The src directory contains C++ code for integrating with proprietary ByteVC1 technology.

## Development Approaches

AnimaX supports two distinct development workflows for different types of contributors:

### Internal Development (ByteDance Developers)

For internal developers, the recommended approach is to set up the template-assembler project development environment first, then submit merge requests to the template-assembler repository. This approach allows development of both open source and proprietary components.

After the commit queue process, the open source components are automatically synchronized to the `lynx-family/animax` GitHub repository.

**Recommended Reading**: [AnimaX Development and Debugging Information](https://bytedance.larkoffice.com/docx/W2iEdNrgDo8QjRxoJorcHmdQnue) for demonstration animations and detailed setup instructions.

### Open Source Development (External Contributors)

For open source developers, clone the `lynx-family/animax` repository from GitHub. The project provides an independent development environment specifically for the open source components.

Detailed development guides are available in the `animax/animax` directory of the repository.

## License

This project is licensed under the Apache 2.0 License. See the LICENSE file for details.