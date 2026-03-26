import { Logger } from './log';
import { AnimaXWebGPUContext } from './gpu';

export enum AnimaXResourceRequestType {
  kLoadRawData = 1,
  kLoadBitmap = 2,
}

interface SuccessData {
  callbackId: number;
  pixelData?: Uint8Array | null;
  width?: number;
  height?: number;
  textureHandle?: number;
}

/**
 * Handles resource loading and forwards results to native callbacks.
 */
export class AnimaXResourceLoader {
  private readonly _module: any;
  private readonly _gpuContext: AnimaXWebGPUContext | null = null;
  private readonly _gpuTextures: Map<number, GPUTexture> | null = null;
  constructor(
    moduleInstance: any,
    gpuContext: AnimaXWebGPUContext | null = null
  ) {
    this._module = moduleInstance;
    this._gpuContext = gpuContext;
    if (this._gpuContext) {
      this._gpuTextures = new Map<number, GPUTexture>();
    }
  }

  destroy(): void {
    this.clearGpuTextures();
  }

  public async loadUrl(
    url: string,
    resultType: number,
    callbackId: number
  ): Promise<void> {
    try {
      let response: Response | null = null;
      let pixelData: Uint8Array | null = null;
      if (url.startsWith('/') || url.startsWith('file://')) {
        // TODO(aiyongbiao.rick): load local file
      } else {
        response = await fetch(url);
        if (!response.ok) {
          this.onLoadError(
            callbackId,
            `Fetch url failed: ${response.status} ${response.statusText}`
          );
          return;
        }
      }
      if (resultType === AnimaXResourceRequestType.kLoadRawData) {
        await this.loadRawData(pixelData, response, callbackId);
      } else if (resultType === AnimaXResourceRequestType.kLoadBitmap) {
        await this.loadBitmap(pixelData, response, callbackId);
      } else {
        this.onLoadError(
          callbackId,
          `Unknown resource request type: ${resultType}`
        );
      }
    } catch (error) {
      const msg = error instanceof Error ? error.message : String(error);
      this.onLoadError(callbackId, `Resource loading failed: ${msg}`);
    }
  }

  private onLoadError(callbackId: number, errorMessage: string) {
    this._module.AnimaXWasm.onResourceLoadError(callbackId, errorMessage);
    Logger.error(`Resource load error: ${errorMessage}`);
  }

  private onLoadSuccess(successData: SuccessData) {
    const {
      callbackId,
      pixelData = null,
      width = 0,
      height = 0,
      textureHandle = 0,
    } = successData;
    try {
      this._module.AnimaXWasm.onResourceLoadSuccess(
        callbackId,
        pixelData,
        width,
        height,
        textureHandle
      );
    } catch (error) {
      const msg = error instanceof Error ? error.message : String(error);
      Logger.error(`Failed to call onResourceLoadSuccess: ${msg}`);
    }
  }

  private async loadRawData(
    pixelData: Uint8Array | null,
    response: Response | null,
    callbackId: number
  ): Promise<void> {
    if (!pixelData && response) {
      pixelData = new Uint8Array(await response.arrayBuffer());
    }
    if (pixelData.byteLength === 0) {
      this.onLoadError(callbackId, 'Downloaded data is empty');
    } else {
      this.onLoadSuccess({
        callbackId,
        pixelData,
      });
    }
  }

  private async loadBitmap(
    pixelData: Uint8Array | null,
    response: Response | null,
    callbackId: number
  ): Promise<void> {
    const blob = response
      ? await response.blob()
      : pixelData
      ? new Blob([(pixelData as unknown) as BlobPart])
      : null;
    if (!blob || blob.size === 0) {
      this.onLoadError(callbackId, 'Downloaded blob is null or empty');
      return;
    }
    const imageBitmap = await createImageBitmap(blob);
    try {
      // Prefer GPU path if available; fallback to CPU RGBA path.
      if (this._gpuContext) {
        const ok = await this.loadBitmapAsGPUTexture(imageBitmap, callbackId);
        if (ok) return;
      }
      await this.loadBitmapRGBA(imageBitmap, callbackId);
    } finally {
      // Always close ImageBitmap to release resources.
      try {
        imageBitmap.close();
      } catch {
        // ignore
      }
    }
  }

  private async loadBitmapAsGPUTexture(
    imageBitmap: ImageBitmap,
    callbackId: number
  ): Promise<boolean> {
    try {
      const { handle: textureHandle, texture } = this._gpuContext.createTexture(
        imageBitmap
      );
      // Do not rely on truthy check; handle validity should be explicit.
      // If your convention is "0 means invalid", this check is correct.
      if (textureHandle !== 0) {
        this._gpuTextures.set(textureHandle, texture);
        this.onLoadSuccess({
          callbackId,
          textureHandle,
          width: imageBitmap.width,
          height: imageBitmap.height,
        });
        return true;
      }
      Logger.warn(
        'createTexture returned an invalid handle, fallback to canvas decoding'
      );
    } catch (error) {
      const msg = error instanceof Error ? error.message : String(error);
      Logger.warn(
        `Failed to create texture: ${msg}, fallback to canvas decoding`
      );
    }
    return false;
  }

  private async loadBitmapRGBA(
    imageBitmap: ImageBitmap,
    callbackId: number
  ): Promise<boolean> {
    let canvas: OffscreenCanvas | HTMLCanvasElement;
    if (typeof OffscreenCanvas !== 'undefined') {
      canvas = new OffscreenCanvas(imageBitmap.width, imageBitmap.height);
    } else {
      canvas = document.createElement('canvas');
      canvas.width = imageBitmap.width;
      canvas.height = imageBitmap.height;
    }
    const ctx = canvas.getContext('2d');
    if (!ctx) {
      this.onLoadError(callbackId, 'Failed to get 2D context');
      return false;
    }
    ctx.drawImage(imageBitmap, 0, 0);
    const imageData = ctx.getImageData(
      0,
      0,
      imageBitmap.width,
      imageBitmap.height
    );
    const pixelData = new Uint8Array(imageData.data);
    if (pixelData.length === 0) {
      this.onLoadError(callbackId, 'Decoded bitmap data is empty');
      return false;
    }
    this.onLoadSuccess({
      callbackId,
      pixelData,
      width: imageBitmap.width,
      height: imageBitmap.height,
    });
    return true;
  }

  private clearGpuTextures() {
    if (!this._gpuTextures || this._gpuTextures.size === 0) {
      return;
    }
    this._gpuTextures.forEach((texture) => {
      try {
        texture.destroy();
      } catch {
        // Ignore errors from double-destroy or backend-specific behavior
      }
    });
    this._gpuTextures.clear();
  }
}
