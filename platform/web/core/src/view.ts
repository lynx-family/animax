import animax_wasm from '../public/build/animax_wasm.js';
import { AnimaXResourceLoader } from './resource.js';
import { AnimaXWebGPUContext } from './gpu.js';
import { Logger } from './log.js';
import {
  AnimaXViewProps,
  AnimaXViewController,
  AnimaXValueParam,
  AnimaXPropertyCallback,
  AnimaXPoint,
  AnimaXFontConfig,
  AnimaXFontDescriptor,
  AnimaXLayerPropertyType,
  AnimaXResourcePropertyType,
  AnimaXLayerBoundsSpace,
} from './view-types.js';

const SafeHTMLElement =
  typeof HTMLElement !== 'undefined'
    ? HTMLElement
    : (class {} as typeof HTMLElement);

const ATTRIBUTE_PRIORITY_HIGH = -100;
const ATTRIBUTE_PRIORITY_LOW = 100;
const FRAME_CAPTURE_TIMEOUT_MS = 10_000;

type PendingAttributeTask = {
  name: string;
  value: string;
  priority: number;
  order: number;
};

type NormalizedFontDescriptor = {
  family: string;
  url: string;
  isDefault: boolean;
  fallbackPriority: number;
};

type CapturedFrame = {
  pixels: Uint8ClampedArray;
  width: number;
  height: number;
};

class AnimaXViewElement
  extends SafeHTMLElement
  implements AnimaXViewProps, AnimaXViewController {
  // --- DOM attributes (props) ---
  public backend: 'webgl' | 'webgpu' = 'webgl';
  public src: string = '';
  public 'src-format': string = '';
  public 'src-polyfill': Record<string, string> | string = '';
  public json: string = '';
  public speed: number = 1.0;
  public autoplay: boolean = true;
  public 'start-frame': number = 0;
  public 'end-frame': number = -1;
  public 'auto-reverse': boolean = false;
  public progress: number = 0;
  public loop: boolean = false;
  public 'loop-count': number = 1;
  public objectfit: 'contain' | 'cover' | 'center' | 'fill' | 'scale-down' =
    'contain';
  public 'object-position':
    | 'center'
    | 'left'
    | 'right'
    | 'top'
    | 'bottom'
    | 'top-left'
    | 'top-right'
    | 'bottom-left'
    | 'bottom-right' = 'center';
  public 'fps-event-interval': number = 0;
  public 'max-frame-rate': number = 60;
  public 'dynamic-resource': boolean = false;
  public 'keeplastframe': boolean = true;
  // --- Static members ---
  private static _instanceCount = 0;
  private static _fontLoadPromises = new Map<string, Promise<boolean>>();
  private static _loadVideoModulePromise: Promise<boolean> | null = null;
  private static _loadTextraModulePromise: Promise<boolean> | null = null;
  private static _dynamicModuleUrls = new Map<string, string>();
  private static _locateFileConfigured = false;
  private static _moduleInstance: any = null;
  private static _getAnimaxModulePromise: Promise<any> | null = null;
  private static _webgpuContext: AnimaXWebGPUContext | null = null;
  // --- Internal members ---
  private _canvas: HTMLCanvasElement | null = null;
  private _module: any = null;
  private _wasm: any = null;
  private _player: any = null;
  private _activeWebGPUContext: AnimaXWebGPUContext | null = null;
  private _glContextHandle: number = 0;
  private _resourceLoader: AnimaXResourceLoader | null = null;
  private _visibilityFlag: number = 0;
  private _intersectionObserver?: IntersectionObserver;
  private _mutationObserver?: MutationObserver;
  private _resizeObserver?: ResizeObserver;
  private _pendingDimensionUpdate = false;
  private _pendingAttributeFlush = false;
  private _attributeTaskOrder = 0;
  private _pendingAttributeTasks = new Map<string, PendingAttributeTask>();
  private _initGeneration = 0;
  private _animationSourceIdentity: string | null = null;
  private _capturedFrame: CapturedFrame | null = null;
  private _frameCaptureGeneration = 0;
  private _captureAfterNextFirstFrame = false;
  private _frameCapturePromise: Promise<CapturedFrame | null> | null = null;
  private _resolveFrameCapture:
    | ((frame: CapturedFrame | null) => void)
    | null = null;
  private _frameCaptureTimeout: ReturnType<typeof setTimeout> | null = null;
  static get observedAttributes() {
    return [
      'backend',
      'src',
      'src-format',
      'src-polyfill',
      'width',
      'height',
      'json',
      'speed',
      'autoplay',
      'start-frame',
      'end-frame',
      'auto-reverse',
      'progress',
      'loop',
      'loop-count',
      'objectfit',
      'object-position',
      'fps-event-interval',
      'max-frame-rate',
      'dynamic-resource',
      'keeplastframe',
    ];
  }
  // --- Playback control ---
  public play(): void {
    this._player?.play();
  }
  public pause(): void {
    this._player?.pause();
  }
  public resume(): void {
    this._player?.resume();
  }
  public stop(): void {
    this._player?.stop();
  }
  public reload(): void {
    this._player?.reload();
  }
  public async captureFrame(
    type: string = 'image/png',
    quality?: number
  ): Promise<Blob | null> {
    if (!this.isFrameCaptureEnabled() || this._activeWebGPUContext) return null;
    const frame = this._capturedFrame ?? (await this._frameCapturePromise);
    if (!frame) return null;
    this._capturedFrame = null;
    this._frameCapturePromise = null;
    return this.encodeCapturedFrame(frame, type, quality);
  }
  public requestFrameCapture(): void {
    if (
      !this.isFrameCaptureEnabled() ||
      !this._wasm ||
      this._activeWebGPUContext
    ) {
      return;
    }
    this._captureAfterNextFirstFrame = false;
    this.resetFrameCaptureState();
    this.configureFrameCapture();
    this._wasm.requestFrameCapture();
  }
  public seek(frame: number): void {
    this._player?.seek(frame);
  }
  public playSegment(startFrame: number, endFrame: number): void {
    this._player?.playSegment(startFrame, endFrame);
  }
  public isAnimating(): boolean {
    return this._player?.isAnimating() ?? false;
  }
  public getDuration(): number {
    return this._player?.getDurationMs() ?? 0;
  }
  public getCurrentFrame(): number {
    return this._player?.getCurrentFrame() ?? 0;
  }
  // --- Playback configuration ---
  public setLoop(loop: boolean): void {
    this._player?.setLoop(loop);
  }
  public setLoopCount(loopCount: number): void {
    this._player?.setLoopCount(loopCount);
  }
  public setProgress(progress: number): void {
    this._player?.setProgress(progress);
  }
  public setAutoPlay(autoplay: boolean): void {
    this._player?.setAutoPlay(autoplay);
  }
  public setSpeed(speed: number): void {
    this._player?.setSpeed(speed);
  }
  public setStartFrame(startFrame: number): void {
    this._player?.setStartFrame(startFrame);
  }
  public setEndFrame(endFrame: number): void {
    this._player?.setEndFrame(endFrame);
  }
  public setAutoReverse(autoReverse: boolean): void {
    this._player?.setAutoReverse(autoReverse);
  }
  public setKeepLastFrame(keepLastFrame: boolean): void {
    this._player?.setKeepLastFrame(keepLastFrame);
  }
  public setObjectFit(objectFit: string): void {
    let enumValue: number;
    const normalized = objectFit.toLowerCase().trim();
    switch (normalized) {
      case 'center':
        enumValue = 0;
        break;
      case 'cover':
        enumValue = 1;
        break;
      case 'fill':
        enumValue = 3;
        break;
      case 'scale-down':
        enumValue = 4;
        break;
      case 'contain':
      default:
        enumValue = 2;
        break;
    }
    this._player?.setObjectFit(enumValue);
  }
  public setObjectPosition(objectPosition: string): void {
    let enumValue: number;
    const normalized = objectPosition.toLowerCase().trim();
    switch (normalized) {
      case 'left':
        enumValue = 1;
        break;
      case 'right':
        enumValue = 2;
        break;
      case 'top':
        enumValue = 3;
        break;
      case 'bottom':
        enumValue = 4;
        break;
      case 'top-left':
        enumValue = 5;
        break;
      case 'top-right':
        enumValue = 6;
        break;
      case 'bottom-left':
        enumValue = 7;
        break;
      case 'bottom-right':
        enumValue = 8;
        break;
      case 'center':
      default:
        enumValue = 0;
        break;
    }
    this._player?.setObjectPosition(enumValue);
  }
  public setFpsEventInterval(interval: number): void {
    this._player?.setFpsEventInterval(interval);
  }
  public setMaxFrameRate(maxFrameRate: number): void {
    this._wasm?.setMaxFrameRate(maxFrameRate);
  }
  public setDynamicResource(dynamic: boolean): void {
    this._player?.setDynamicResource(dynamic);
  }
  // --- Animation source & resources ---
  public setJson(json: string): void {
    this.setAnimationSource('json', json);
  }
  public setImageFolder(imageFolder: string): void {
    this._player?.setImageFolder(imageFolder);
  }
  public setSrc(src: string): void {
    this.setAnimationSource('src', src);
  }
  public setSrcPolyfill(polyfill: Record<string, string> | string): void {
    if (typeof polyfill === 'string') {
      try {
        this._player?.setSrcPolyfill(JSON.parse(polyfill));
      } catch {
        this._player?.setSrcPolyfill({});
      }
    } else {
      this._player?.setSrcPolyfill(polyfill || {});
    }
  }
  // --- Interaction and Events ---
  public subscribeUpdateEvent(frame: number): void {
    this._player?.subscribeUpdateEvent(frame);
  }
  public unsubscribeUpdateEvent(frame: number): void {
    this._player?.unsubscribeUpdateEvent(frame);
  }
  public subscribeUpdateEvents(frames: number[]): void {
    this._player?.subscribeUpdateEvents(frames, true);
  }
  public unsubscribeUpdateEvents(frames: number[]): void {
    this._player?.subscribeUpdateEvents(frames, false);
  }
  // --- Property update ---
  public setResourceProperty(
    resource_type: AnimaXResourcePropertyType,
    resource_id: string,
    value: AnimaXValueParam,
    callback?: AnimaXPropertyCallback
  ): void {
    const propertyCallback = callback ?? (() => {});
    this._wasm?.setResourceProperty(
      resource_type,
      resource_id,
      value,
      propertyCallback
    );
  }
  public updateLayerProperty(
    layer_type: AnimaXLayerPropertyType,
    layer_name: string,
    value: AnimaXValueParam,
    callback?: AnimaXPropertyCallback
  ): void {
    const propertyCallback = callback ?? (() => {});
    this.autoAdaptValueParam(layer_type, value);
    this._wasm?.updateLayerProperty(
      layer_type,
      layer_name,
      value,
      propertyCallback
    );
  }

  public getLayerBounds(
    layerName: string,
    boundsSpace: AnimaXLayerBoundsSpace,
    callback?: (
      success: boolean,
      x: number,
      y: number,
      width: number,
      height: number
    ) => void
  ): void {
    // Validate layer_name must not be empty
    if (!layerName || layerName.length === 0) {
      if (callback) {
        callback(false, 0, 0, 0, 0);
      }
      return;
    }

    const boundsCallback = callback ?? (() => {});

    // Call WASM binding
    this._wasm?.getLayerBounds(
      layerName,
      boundsSpace,
      (
        success: boolean,
        x: number,
        y: number,
        width: number,
        height: number
      ) => {
        boundsCallback(success, x, y, width, height);
      }
    );
  }

  public updateTextSizeByLayerName(
    layerName: string,
    textSize: number,
    targetFrame?: number,
    callback?: AnimaXPropertyCallback
  ): void {
    this.updateLayerProperty(
      AnimaXLayerPropertyType.TextSize,
      layerName,
      createAnimaXValueParam(textSize, targetFrame),
      callback
    );
  }
  public updateTextColorByLayerName(
    layerName: string,
    textColor: string,
    targetFrame?: number,
    callback?: AnimaXPropertyCallback
  ): boolean {
    const hexTextColor = this.formatColorString(textColor);
    if (hexTextColor === null) {
      return false;
    }
    this.updateLayerProperty(
      AnimaXLayerPropertyType.TextColor,
      layerName,
      createAnimaXValueParam(hexTextColor, targetFrame),
      callback
    );
    return true;
  }
  public updateTextByLayerName(
    layerName: string,
    newText: string,
    targetFrame?: number,
    callback?: AnimaXPropertyCallback
  ): void {
    this.updateLayerProperty(
      AnimaXLayerPropertyType.TextValue,
      layerName,
      createAnimaXValueParam(newText, targetFrame),
      callback
    );
  }
  public updateImageById(imageId: string, newImageUrl: string): void {
    this.setResourceProperty(
      AnimaXResourcePropertyType.ImageDirName,
      imageId,
      createAnimaXValueParam('')
    );
    this.setResourceProperty(
      AnimaXResourcePropertyType.ImageFileName,
      imageId,
      createAnimaXValueParam(newImageUrl)
    );
  }
  public updateVideoById(videoId: string, newVideoUrl: string): void {
    this.setResourceProperty(
      AnimaXResourcePropertyType.VideoDirName,
      videoId,
      createAnimaXValueParam('')
    );
    this.setResourceProperty(
      AnimaXResourcePropertyType.VideoFileName,
      videoId,
      createAnimaXValueParam(newVideoUrl)
    );
  }
  public updateFontByName(fontName: string, newFontPath: string): void {
    this.setResourceProperty(
      AnimaXResourcePropertyType.FontPath,
      fontName,
      createAnimaXValueParam(newFontPath)
    );
  }
  /**
   * Configure the global wasm font registry.
   *
   * Fonts are shared by all AnimaX instances because the current wasm font
   * manager is process-global.
   */
  public static async configureFonts(
    config: AnimaXFontConfig
  ): Promise<boolean> {
    if (!config || !Array.isArray(config.fonts)) {
      Logger.error('failed to configure fonts, fonts must be an array');
      return false;
    }

    const defaultFamily = config.defaultFamily ?? '';
    const results = await Promise.all(
      config.fonts.map((font, index) => {
        const normalized = this.normalizeFontDescriptor(
          font,
          index,
          defaultFamily
        );
        return this.loadNormalizedFont(normalized);
      })
    );
    return results.every(Boolean);
  }

  /**
   * Load a list of fallback fonts into the global wasm font registry.
   */
  public static async loadFonts(
    fonts: AnimaXFontDescriptor[]
  ): Promise<boolean> {
    return this.configureFonts({ fonts });
  }

  /**
   * Load one fallback font into the global wasm font registry.
   */
  public static async loadFont(font: AnimaXFontDescriptor): Promise<boolean> {
    const normalized = this.normalizeFontDescriptor(font, 0, '');
    return this.loadNormalizedFont(normalized);
  }

  private static async loadNormalizedFont(
    normalized: NormalizedFontDescriptor
  ): Promise<boolean> {
    const cacheKey = this.getFontCacheKey(normalized);
    const cached = this._fontLoadPromises.get(cacheKey);
    if (cached) {
      return cached;
    }

    const promise = this.loadFontInternal(normalized).then(
      (result) => {
        if (!result) {
          this._fontLoadPromises.delete(cacheKey);
        }
        return result;
      },
      (error) => {
        this._fontLoadPromises.delete(cacheKey);
        throw error;
      }
    );
    this._fontLoadPromises.set(cacheKey, promise);
    return promise;
  }
  /**
   * Load default video module for all instances (singleton)
   */
  public static async loadVideoModule(
    url: string | null = null
  ): Promise<boolean> {
    if (this._loadVideoModulePromise) {
      return this._loadVideoModulePromise;
    }

    if (url === null) {
      Logger.error('failed to load video module, url is null');
      return false;
    }

    this._loadVideoModulePromise = this._loadDynamicModule(
      'animax_video.wasm',
      url,
      'video'
    );

    return this._loadVideoModulePromise;
  }

  /**
   * Load default Textra text-layout module for all instances (singleton)
   */
  public static async loadTextraModule(
    url: string | null = null
  ): Promise<boolean> {
    if (this._loadTextraModulePromise) {
      return this._loadTextraModulePromise;
    }

    if (url === null) {
      Logger.error('failed to load textra module, url is null');
      return false;
    }

    this._loadTextraModulePromise = this._loadDynamicModule(
      'animax_textra.wasm',
      url,
      'textra'
    );

    return this._loadTextraModulePromise;
  }

  // --- Lifecycle ---
  constructor() {
    super();
  }

  connectedCallback() {
    window.addEventListener('beforeunload', this.clearPlayerAndListeners);
    this.setupVisibilityListeners();
    void this.initPlayer();
  }
  disconnectedCallback() {
    window.removeEventListener('beforeunload', this.clearPlayerAndListeners);
    Logger.info('clearPlayerAndListeners');
    this.clearPlayerAndListeners();
  }
  attributeChangedCallback(name: string, oldValue: string, newValue: string) {
    if (oldValue === newValue) return;
    if (name === 'backend') {
      if (this._player) {
        Logger.info('backend changed, destroy player and create new player');
        this.restartPlayer();
      }
    } else if (name === 'width' || name === 'height') {
      this.scheduleDimensionChange();
    } else {
      if (this.shouldDeferAttribute(name)) {
        this.enqueueAttributeUpdate(name, newValue);
      } else {
        this.handleAttribute(name, newValue);
      }
    }
  }
  // --- Player Initialization ---
  private async initPlayer() {
    const generation = ++this._initGeneration;
    Logger.info('initPlayer');
    const module = await AnimaXViewElement._getAnimaxModule();
    if (!this.isInitializationCurrent(generation)) {
      return;
    }
    const activeWebGPUContext = await this.getWebGPUContext();
    if (!this.isInitializationCurrent(generation)) {
      return;
    }

    this._module = module;
    if (this._resourceLoader) {
      this._resourceLoader.destroy();
    }
    this._activeWebGPUContext = activeWebGPUContext;
    this._resourceLoader = new AnimaXResourceLoader(
      this._module,
      this._activeWebGPUContext
    );
    const pixelRatio = this.getPixelRatio();
    this._wasm = new this._module.AnimaXWasm(pixelRatio);
    this._player = this._wasm.getPlayer();
    this._wasm.setResourceLoaderImpl(
      this._resourceLoader.loadUrl.bind(this._resourceLoader)
    );

    this._wasm.setEventCallback((eventName: string, params: any) => {
      Logger.info(
        `dispatch event ${eventName} with params ${JSON.stringify(params)}`
      );
      if (eventName === 'firstframe' && this._captureAfterNextFirstFrame) {
        this._captureAfterNextFirstFrame = false;
        this.requestFrameCapture();
      }
      this.dispatchEvent(
        new CustomEvent(eventName, {
          detail: params,
          bubbles: false,
        })
      );
    });

    this.configureFrameCapture();

    this.ensureInnerCanvas();
    this.bindCanvasContextEvents();
    this.handleDimensionChange(true);
    this.bindCanvasTapEvent();
    this._pendingAttributeTasks.clear();
    for (let i = 0; i < this.attributes.length; i++) {
      const attr = this.attributes[i];
      if (this.shouldDeferAttribute(attr.name)) {
        this.enqueueAttributeUpdate(attr.name, attr.value);
      } else {
        this.handleAttribute(attr.name, attr.value);
      }
    }
    this.flushAttributeUpdates();
  }

  private isInitializationCurrent(generation: number): boolean {
    return generation === this._initGeneration && this.isConnected;
  }

  private restartPlayer = () => {
    this._initGeneration++;
    this.destroyPlayer();
    this.removeInnerCanvas();
    if (this.isConnected) {
      void this.initPlayer();
    }
  };

  private destroyPlayer() {
    this._captureAfterNextFirstFrame = false;
    this.resetFrameCaptureState();
    this.unbindCanvasContextEvents();
    if (this._player) {
      this.unbindCanvasTapEvent();
      this._player.stop();
      this._player.delete();
      this._player = null;
    }
    if (this._resourceLoader) {
      this._resourceLoader.destroy();
      this._resourceLoader = null;
    }
    this._activeWebGPUContext = null;
    if (this._wasm) {
      this._wasm.delete();
      this._wasm = null;
    }
    this._glContextHandle = 0;
  }

  private removeInnerCanvas() {
    this._canvas?.remove();
    this._canvas = null;
  }
  // --- Canvas management ---
  private ensureInnerCanvas() {
    if (this._canvas) return;
    AnimaXViewElement._instanceCount++;
    this._canvas = document.createElement('canvas');
    this._canvas.id = `animax-canvas-${Date.now()}-${Math.floor(
      Math.random() * 10000
    )}-${AnimaXViewElement._instanceCount}`;
    Logger.info(`create canvas ${this._canvas.id}`);
    this._canvas.style.display = 'block';
    this._canvas.style.width = '100%';
    this._canvas.style.height = '100%';
    this.appendChild(this._canvas);
  }

  private bindCanvasContextEvents() {
    this._canvas?.addEventListener(
      'webglcontextlost',
      this.handleWebGLContextLost
    );
    this._canvas?.addEventListener(
      'webglcontextrestored',
      this.handleWebGLContextRestored
    );
  }

  private unbindCanvasContextEvents() {
    this._canvas?.removeEventListener(
      'webglcontextlost',
      this.handleWebGLContextLost
    );
    this._canvas?.removeEventListener(
      'webglcontextrestored',
      this.handleWebGLContextRestored
    );
  }

  private handleWebGLContextLost = (event: Event) => {
    event.preventDefault();
    Logger.warn('WebGL context lost, waiting for context restoration');
    this.dispatchEvent(
      new CustomEvent('renderingunavailable', {
        detail: { reason: 'context-lost' },
        bubbles: false,
      })
    );
  };

  private handleWebGLContextRestored = () => {
    Logger.info('WebGL context restored, recreating player');
    this.restartPlayer();
  };
  private scheduleDimensionChange() {
    if (this._pendingDimensionUpdate) return;
    this._pendingDimensionUpdate = true;
    Promise.resolve().then(() => {
      this._pendingDimensionUpdate = false;
      this.handleDimensionChange();
    });
  }
  private async getWebGPUContext() {
    const backend = this.getAttribute('backend');
    if (backend !== 'webgpu') {
      return null;
    }
    try {
      return await AnimaXViewElement.getWebGPUContext();
    } catch (error) {
      const msg = error instanceof Error ? error.message : String(error);
      Logger.warn(`WebGPU backend unavailable, fallback to WebGL: ${msg}`);
      return null;
    }
  }

  private isFrameCaptureEnabled(): boolean {
    const value = this.getAttribute('capture-frame');
    return value === '' || value === 'true';
  }

  private configureFrameCapture(): void {
    if (!this.isFrameCaptureEnabled() || this._activeWebGPUContext) return;
    const generation = this._frameCaptureGeneration;
    this.clearFrameCaptureTimeout();
    this._frameCapturePromise = new Promise((resolve) => {
      this._resolveFrameCapture = resolve;
    });
    this._frameCaptureTimeout = setTimeout(() => {
      if (generation !== this._frameCaptureGeneration) return;
      this._frameCaptureTimeout = null;
      this._resolveFrameCapture?.(null);
      this._resolveFrameCapture = null;
      this._frameCapturePromise = null;
    }, FRAME_CAPTURE_TIMEOUT_MS);
    this._wasm.setFrameCaptureCallback(
      (view: Uint8Array, width: number, height: number) => {
        if (generation !== this._frameCaptureGeneration) return;
        if (width <= 0 || height <= 0 || view.length !== width * height * 4) {
          return;
        }
        const frame = {
          pixels: new Uint8ClampedArray(view),
          width,
          height,
        };
        this._capturedFrame = frame;
        this.clearFrameCaptureTimeout();
        this._resolveFrameCapture?.(frame);
        this._resolveFrameCapture = null;
        this.dispatchEvent(
          new CustomEvent('frameavailable', {
            detail: {},
            bubbles: false,
          })
        );
      }
    );
  }

  private resetFrameCaptureState(): void {
    this._frameCaptureGeneration++;
    this.clearFrameCaptureTimeout();
    this._resolveFrameCapture?.(null);
    this._resolveFrameCapture = null;
    this._capturedFrame = null;
    this._frameCapturePromise = null;
  }

  private setAnimationSource(type: 'src' | 'json', value: string): void {
    if (!this._player) return;
    if (value) {
      const identity = `${type}:${value}`;
      if (
        this._animationSourceIdentity !== null &&
        identity !== this._animationSourceIdentity
      ) {
        this.resetFrameCaptureState();
        this._captureAfterNextFirstFrame =
          this.isFrameCaptureEnabled() && !this._activeWebGPUContext;
      }
      this._animationSourceIdentity = identity;
    }
    if (type === 'json') {
      this._player.setJson(value);
    } else {
      this._player.setSrc(value);
    }
  }

  private clearFrameCaptureTimeout(): void {
    if (this._frameCaptureTimeout !== null) {
      clearTimeout(this._frameCaptureTimeout);
      this._frameCaptureTimeout = null;
    }
  }

  private encodeCapturedFrame(
    frame: CapturedFrame,
    type: string,
    quality?: number
  ): Promise<Blob | null> {
    const rowBytes = frame.width * 4;
    const flipped = new Uint8ClampedArray(frame.pixels.length);
    for (let row = 0; row < frame.height; row++) {
      const sourceOffset = (frame.height - row - 1) * rowBytes;
      flipped.set(
        frame.pixels.subarray(sourceOffset, sourceOffset + rowBytes),
        row * rowBytes
      );
    }
    const image = new ImageData(flipped, frame.width, frame.height);
    if (typeof OffscreenCanvas !== 'undefined') {
      const output = new OffscreenCanvas(frame.width, frame.height);
      const context = output.getContext('2d');
      if (!context) return Promise.resolve(null);
      context.putImageData(image, 0, 0);
      return output.convertToBlob({ type, quality });
    }

    const output = document.createElement('canvas');
    output.width = frame.width;
    output.height = frame.height;
    const context = output.getContext('2d');
    if (!context) return Promise.resolve(null);
    context.putImageData(image, 0, 0);
    return new Promise((resolve) => output.toBlob(resolve, type, quality));
  }

  private async handleDimensionChange(forceUpdate: boolean = false) {
    if (!this._canvas) {
      Logger.info('handleDimensionChange failed, canvas not ready');
      return;
    }
    const pixelRatio = this.getPixelRatio();
    const width =
      Number(this.getAttribute('width')) ||
      this.offsetWidth * pixelRatio ||
      300 * pixelRatio;
    const height =
      Number(this.getAttribute('height')) ||
      this.offsetHeight * pixelRatio ||
      150 * pixelRatio;
    if (
      forceUpdate ||
      this._canvas.width !== width ||
      this._canvas.height !== height
    ) {
      this.dispatchEvent(
        new CustomEvent('renderingunavailable', {
          detail: { reason: 'dimension-change' },
          bubbles: false,
        })
      );
      this._canvas.width = width;
      this._canvas.height = height;
      let wasmContext: any = null;
      let glContextHandle: number = 0;

      if (this._activeWebGPUContext) {
        wasmContext = this._activeWebGPUContext.wasmContext;
      } else {
        let gl:
          | WebGL2RenderingContext
          | WebGLRenderingContext
          | null = this._canvas.getContext('webgl2', {
          stencil: true,
          antialias: false,
          powerPreference: 'high-performance',
        });
        // Fallback to WebGL 1 if WebGL 2 is not available
        if (!gl) {
          gl = this._canvas.getContext('webgl', {
            stencil: true,
            antialias: false,
            powerPreference: 'high-performance',
          });
        }
        if (gl && this._module.GL) {
          if (!this._glContextHandle) {
            this._glContextHandle = this._module.GL.registerContext(gl, {
              majorVersion: 2,
              minorVersion: 0,
            });
          }
          glContextHandle = this._glContextHandle;
        } else {
          Logger.info(
            `Failed to create WebGL context in JS, gl: ${gl}, GL: ${this._module.GL}`
          );
        }
      }
      this._wasm.setSurfaceWithCanvas(
        '#' + this._canvas.id,
        width,
        height,
        wasmContext,
        glContextHandle
      );
    }
  }
  private shouldDeferAttribute(name: string): boolean {
    switch (name) {
      case 'dynamic-resource':
      case 'src-polyfill':
      case 'json':
      case 'src':
      case 'src-format':
        return true;
      default:
        return false;
    }
  }
  private getAttributePriority(name: string): number {
    switch (name) {
      case 'dynamic-resource':
      case 'src-polyfill':
        return ATTRIBUTE_PRIORITY_HIGH;
      case 'json':
      case 'src':
      case 'src-format':
        return ATTRIBUTE_PRIORITY_LOW;
    }
    return ATTRIBUTE_PRIORITY_LOW;
  }
  private enqueueAttributeUpdate(name: string, value: string): void {
    const task: PendingAttributeTask = {
      name,
      value,
      priority: this.getAttributePriority(name),
      order: this._attributeTaskOrder++,
    };
    this._pendingAttributeTasks.set(name, task);
    this.scheduleAttributeFlush();
  }
  private scheduleAttributeFlush(): void {
    if (this._pendingAttributeFlush) return;
    this._pendingAttributeFlush = true;
    Promise.resolve().then(() => {
      this._pendingAttributeFlush = false;
      this.flushAttributeUpdates();
    });
  }
  private flushAttributeUpdates(): void {
    if (!this._player || this._pendingAttributeTasks.size === 0) {
      return;
    }
    const tasks = Array.from(this._pendingAttributeTasks.values()).sort(
      (left, right) =>
        left.priority - right.priority || left.order - right.order
    );
    this._pendingAttributeTasks.clear();
    for (const task of tasks) {
      this.handleAttribute(task.name, task.value);
    }
  }
  // --- Attribute Handling ---
  private handleAttribute(name: string, value: string) {
    if (!this._player) return;
    switch (name) {
      case 'src':
      case 'src-format':
        this.setAnimationSource('src', value);
        break;
      case 'src-polyfill':
        if (typeof value === 'string') {
          try {
            this._player.setSrcPolyfill(JSON.parse(value));
          } catch {
            this._player.setSrcPolyfill({});
          }
        } else {
          this._player.setSrcPolyfill(value || {});
        }
        break;
      case 'json':
        this.setAnimationSource('json', value);
        break;
      case 'backend':
      case 'width':
      case 'height':
        // handled in attributeChangedCallback
        break;
      case 'loop':
        this._player.setLoop(value === 'true' || value === '');
        break;
      case 'autoplay':
        this._player.setAutoPlay(value === 'true' || value === '');
        break;
      case 'speed':
        this._player.setSpeed(parseFloat(value) || 1);
        break;
      case 'start-frame':
        this._player.setStartFrame(parseInt(value) || 0);
        break;
      case 'end-frame':
        this._player.setEndFrame(parseInt(value) || -1);
        break;
      case 'auto-reverse':
        this._player.setAutoReverse(value === 'true' || value === '');
        break;
      case 'progress':
        this._player.setProgress(parseFloat(value) || 0);
        break;
      case 'loop-count':
        this._player.setLoopCount(parseInt(value) || 1);
        break;
      case 'objectfit':
        this.setObjectFit(value);
        break;
      case 'object-position':
        this.setObjectPosition(value);
        break;
      case 'fps-event-interval':
        this._player.setFpsEventInterval(parseInt(value) || 0);
        break;
      case 'max-frame-rate':
        this._wasm?.setMaxFrameRate(parseInt(value) || 60);
        break;
      case 'dynamic-resource':
        this._player.setDynamicResource(value === 'true' || value === '');
        break;
      case 'keeplastframe':
        this._player.setKeepLastFrame(value === 'true' || value === '');
        break;
      default:
        break;
    }
  }
  // --- Canvas tap event binding ---
  private bindCanvasTapEvent(): void {
    const handleTap = (event: MouseEvent) => {
      const rect = this._canvas!.getBoundingClientRect();
      const scale = this.getPixelRatio();
      const x = (event.clientX - rect.left) * scale;
      const y = (event.clientY - rect.top) * scale;
      this._player.onTap(x, y);
    };
    this._canvas.addEventListener('click', handleTap);
    (this._canvas as any)._handleTap = handleTap;
  }
  private unbindCanvasTapEvent(): void {
    const handleTap = (this._canvas as any)._handleTap;
    if (handleTap) {
      this._canvas.removeEventListener('click', handleTap);
      delete (this._canvas as any)._handleTap;
    }
  }
  // --- Visibility listeners ---
  private clearPlayerAndListeners = () => {
    this._initGeneration++;
    this.destroyPlayer();
    this.removeInnerCanvas();
    this.clearVisibilityListeners();
  };
  private setupVisibilityListeners() {
    // Listen for page visibility changes
    document.addEventListener(
      'visibilitychange',
      this.updateVisibilityStateHandler
    );
    // Listen for intersection (element enters or leaves the viewport)
    this._intersectionObserver = new IntersectionObserver((entries) => {
      // Trigger only when intersection state changes
      if (entries[0].isIntersecting !== undefined) {
        this.updateVisibilityStateHandler();
      }
    });
    this._intersectionObserver.observe(this);
    // Listen for element size changes
    this._resizeObserver = new ResizeObserver(() => {
      this.updateVisibilityStateHandler();
      this.handleDimensionChange();
    });
    this._resizeObserver.observe(this);
    // Listen for element being added to or removed from its parent
    if (this.parentNode) {
      this._mutationObserver = new MutationObserver((mutations) => {
        for (const mutation of mutations) {
          if (
            Array.from(mutation.addedNodes).includes(this) ||
            Array.from(mutation.removedNodes).includes(this)
          ) {
            this.updateVisibilityStateHandler();
            break;
          }
        }
      });
      this._mutationObserver.observe(this.parentNode, {
        childList: true,
      });
    }
  }
  private clearVisibilityListeners() {
    // Remove page visibility change listener
    document.removeEventListener(
      'visibilitychange',
      this.updateVisibilityStateHandler
    );
    // Disconnect intersection observer
    this._intersectionObserver?.disconnect();
    // Disconnect resize observer
    this._resizeObserver?.disconnect();
    // Disconnect mutation observer
    this._mutationObserver?.disconnect();
  }

  private updateVisibilityStateHandler = () => {
    if (!this._player) {
      return;
    }

    const VisibilityState = {
      Background: 1,
      Visible: 2,
      Opacity: 4,
      Size: 8,
      Attach: 16,
    };
    let state = 0;
    if (document.visibilityState !== 'visible') {
      state |= VisibilityState.Background;
    }
    if (
      (this as any).checkVisibility ? (this as any).checkVisibility() : true
    ) {
      // Visible, do nothing (this is a positive flag, not a negative one)
    } else {
      state |= VisibilityState.Visible;
    }
    if (parseFloat(getComputedStyle(this).opacity) < 0.01) {
      state |= VisibilityState.Opacity;
    }
    if (this.offsetWidth === 0 || this.offsetHeight === 0) {
      state |= VisibilityState.Size;
    }
    if (!this.isConnected) {
      state |= VisibilityState.Attach;
    }
    if (this._visibilityFlag === state) {
      return;
    }
    this._visibilityFlag = state;
    Logger.info(`update visibility states to ${state}`);
    this._wasm?.updateVisibilityStates(state);
  };

  private static async loadFontInternal(
    font: NormalizedFontDescriptor
  ): Promise<boolean> {
    if (!font.family || !font.url) {
      Logger.error('failed to load font, family and url are required');
      return false;
    }

    const response = await fetch(font.url);
    if (!response.ok) {
      Logger.error(`failed to load font ${font.family}, url: ${font.url}`);
      return false;
    }

    const arrayBuffer = await response.arrayBuffer();
    if (arrayBuffer.byteLength <= 0) {
      Logger.error(`failed to load font ${font.family}, data is empty`);
      return false;
    }

    const data = new Uint8Array(arrayBuffer);
    const animax = await this._getAnimaxModule();

    if (typeof animax.AnimaXWasm.registerFontWithData === 'function') {
      const result = await animax.AnimaXWasm.registerFontWithData(
        font.family,
        font.isDefault,
        font.fallbackPriority,
        data
      );
      return result;
    }

    Logger.error('failed to register font, wasm binding is unavailable');
    return false;
  }

  private static normalizeFontDescriptor(
    font: AnimaXFontDescriptor,
    index: number,
    defaultFamily: string
  ): NormalizedFontDescriptor {
    const normalizedFamily = this.normalizeFontFamily(font.family);
    return {
      family: font.family.trim(),
      url: font.url.trim(),
      isDefault: normalizedFamily === this.normalizeFontFamily(defaultFamily),
      fallbackPriority: index,
    };
  }

  private static normalizeFontFamily(family: string): string {
    return family.trim().toLowerCase();
  }

  private static getFontCacheKey(font: NormalizedFontDescriptor): string {
    return [font.url, this.normalizeFontFamily(font.family)].join('|');
  }

  private static async _getAnimaxModule(): Promise<any> {
    if (AnimaXViewElement._moduleInstance) {
      return AnimaXViewElement._moduleInstance;
    }

    if (AnimaXViewElement._getAnimaxModulePromise) {
      return AnimaXViewElement._getAnimaxModulePromise;
    }

    AnimaXViewElement._getAnimaxModulePromise = (async () => {
      AnimaXViewElement._moduleInstance = await animax_wasm().catch(
        (error: any) => {
          Logger.error('Failed to load animax-wasm binding:', error);
          throw error;
        }
      );
      return AnimaXViewElement._moduleInstance;
    })();

    return AnimaXViewElement._getAnimaxModulePromise;
  }

  private static async _loadDynamicModule(
    moduleName: string,
    url: string,
    moduleLabel: string
  ): Promise<boolean> {
    try {
      const animax = await this._getAnimaxModule();
      this._dynamicModuleUrls.set(moduleName, url);
      this._configureLocateFile(animax);
      await animax.loadDynamicLibrary(moduleName, {
        globalAsync: true,
        loadAsync: true,
        nodelete: true,
        global: true,
      });
      Logger.info(`load ${moduleLabel} module successfully, url: ${url}`);
      return true;
    } catch (error) {
      Logger.error(`failed to load ${moduleLabel} module:`, error);
      return false;
    }
  }

  private static _configureLocateFile(animax: any): void {
    if (this._locateFileConfigured) {
      return;
    }

    const fallbackLocateFile =
      typeof animax.locateFile === 'function'
        ? animax.locateFile.bind(animax)
        : null;

    animax.locateFile = (path: string, scriptDirectory: string) => {
      if (path === 'animax_wasm.wasm') {
        return new URL('../public/build/animax_wasm.wasm', import.meta.url)
          .href;
      }

      const moduleUrl = this._dynamicModuleUrls.get(path);
      if (moduleUrl) {
        return moduleUrl;
      }

      if (fallbackLocateFile) {
        return fallbackLocateFile(path, scriptDirectory);
      }

      return scriptDirectory + path;
    };

    this._locateFileConfigured = true;
  }

  private static async getWebGPUContext() {
    if (!AnimaXViewElement._webgpuContext) {
      const animax = await this._getAnimaxModule();
      AnimaXViewElement._webgpuContext = await AnimaXWebGPUContext.create(
        animax
      );
    }
    return AnimaXViewElement._webgpuContext;
  }

  private autoScaleLayerProperties = new Set<AnimaXLayerPropertyType>([
    AnimaXLayerPropertyType.TransformAnchor,
    AnimaXLayerPropertyType.TransformPosition,
    AnimaXLayerPropertyType.TextSize,
  ]);

  private getPixelRatio(): number {
    const pixelRatio =
      typeof window !== 'undefined' ? window.devicePixelRatio : 1;
    return Number.isFinite(pixelRatio) ? pixelRatio : 1;
  }

  private autoAdaptValueParam(
    layerType: AnimaXLayerPropertyType,
    param: AnimaXValueParam
  ): void {
    const density = this.getPixelRatio();
    if (
      !this.autoScaleLayerProperties.has(layerType) ||
      density === 1 ||
      !param
    ) {
      return;
    }
    // Scale point properties if both are valid numbers
    if (Number.isFinite(param.pointX) && Number.isFinite(param.pointY)) {
      param.pointX! *= density;
      param.pointY! *= density;
      return;
    }
    // Scale double value property if valid
    if (Number.isFinite(param.doubleValue)) {
      param.doubleValue! *= density;
    }
  }

  private formatColorString(color: string): string | null {
    if (
      typeof color !== 'string' ||
      // eslint-disable-next-line regexp/no-unused-capturing-group
      !/^(#|0x)([0-9a-fA-F]{2}){3,4}$/.test(color)
    ) {
      return null;
    }
    // Remove the leading "#" or "0x"
    const hexColor = color.slice(color.startsWith('#') ? 1 : 2);
    return hexColor;
  }

  private truncate(str: string, maxLen: number = 30): string {
    if (str.length <= maxLen) return str;
    return str.slice(0, maxLen) + '...';
  }
}

export function createAnimaXValueParam(
  value: string | number | boolean | AnimaXPoint,
  targetFrame?: number
): AnimaXValueParam {
  const frameIndex = targetFrame ?? -1;
  if (typeof value === 'object' && 'x' in value && 'y' in value) {
    return {
      pointX: value.x,
      pointY: value.y,
      frameIndex,
    };
  } else if (typeof value === 'string') {
    return {
      stringValue: value,
      frameIndex,
    };
  } else if (typeof value === 'number') {
    return {
      doubleValue: value,
      frameIndex,
    };
  } else if (typeof value === 'boolean') {
    return {
      boolValue: value,
      frameIndex,
    };
  } else {
    // No matching type value, return a ValueParam with only frameIndex.
    return { frameIndex };
  }
}

// Register custom element
if (typeof window !== 'undefined' && !customElements.get('animax-view')) {
  customElements.define('animax-view', AnimaXViewElement);
}
export { AnimaXViewElement };
