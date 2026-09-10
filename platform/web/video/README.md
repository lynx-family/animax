# @lynx-js/animax-video

Optional H.264 and H.265 video decoding for AnimaX Web, powered by FFmpeg and
distributed as a WebAssembly side module.

## Install

Keep this package aligned with the core package version:

```bash
npm install @lynx-js/animax @lynx-js/animax-video
```

## Usage

Load the module before creating animations that contain video layers:

```typescript
import { AnimaXViewElement } from '@lynx-js/animax';
import { AnimaXVideoModuleUrl } from '@lynx-js/animax-video';

await AnimaXViewElement.loadVideoModule(AnimaXVideoModuleUrl);
```

Video layers require accessible resource URLs and do not support video assets
inside ZIP archives on the Web platform.

## License

The AnimaX package code is Apache-2.0. The FFmpeg-derived WebAssembly module is
LGPL-2.1-or-later; see
[`THIRD_PARTY_NOTICES.md`](./THIRD_PARTY_NOTICES.md).
