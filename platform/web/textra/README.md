# @lynx-js/animax-textra

Optional Textra text layout support for AnimaX Web, distributed as a
WebAssembly side module.

## Install

Keep this package aligned with the core package version:

```bash
npm install @lynx-js/animax @lynx-js/animax-textra
```

## Usage

Load the module before creating animations that need Textra text layout:

```typescript
import { AnimaXViewElement } from '@lynx-js/animax';
import { AnimaXTextraModuleUrl } from '@lynx-js/animax-textra';

await AnimaXViewElement.loadTextraModule(AnimaXTextraModuleUrl);
```

Configure Web fonts through `AnimaXViewElement.configureFonts` when custom or
multilingual text rendering is required.

## License

Apache-2.0. Third-party notices are provided in
[`THIRD_PARTY_NOTICES.md`](./THIRD_PARTY_NOTICES.md).
