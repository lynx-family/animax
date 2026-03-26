/**
 * Enum for AnimaX dynamic properties related to the layer.
 */
export enum AnimaXLayerPropertyType {
  // Layer visibility
  Visibility = 1,
  // Layer opacity transform
  TransformOpacity = 2,
  // Layer anchor transform
  TransformAnchor = 3,
  // Layer position transform
  TransformPosition = 4,
  // Layer scale transform
  TransformScale = 5,
  // Layer 2D rotation transform
  TransformRotation = 6,
  // Layer skew transform
  TransformSkew = 7,
  // Layer skew angle transform
  TransformSkewAngle = 8,
  // Layer start opacity transform
  TransformStartOpacity = 9,
  // Layer end opacity transform
  TransformEndOpacity = 10,
  // Layer rotation X transform
  TransformRotationX = 11,
  // Layer 3D rotation Y transform
  TransformRotationY = 12,
  // Layer rotation Z transform
  TransformRotationZ = 13,
  // Text layer value
  TextValue = 101,
  // Text layer size transform
  TextSize = 102,
  // Text layer color transform
  TextColor = 103,
  // Text layer tracking transform
  TextTracking = 104,
  // Layer or content color
  Color = 201,
  // Layer or content color filter
  ColorFilter = 202,
  // Layer or content stroke color
  StrokeColor = 203,
  // Layer or content stroke width
  StrokeWidth = 204,
  // Content opacity
  Opacity = 205,
  // Content blur radius
  BlurRadius = 206,
  // Content ellipse size
  EllipseSize = 207,
  // Content rectangle size
  RectangleSize = 208,
  // Content corner radius
  CornerRadius = 209,
  // Content position
  Position = 210,
  // Content repeater copies
  RepeaterCopies = 211,
  // Content repeater offset
  RepeaterOffset = 212,
  // Content polygon star points
  PolystarPoints = 213,
  // Content polygon star rotation
  PolystarRotation = 214,
  // Content polygon star inner radius
  PolystarInnerRadius = 215,
  // Content polygon star outer radius
  PolystarOuterRadius = 216,
  // Content polygon star inner rounded
  PolystarInnerRounded = 217,
  // Content polygon star outer rounded
  PolystarOuterRounded = 218,
  // Content drop shadow color
  DropShadowColor = 219,
  // Content drop shadow opacity
  DropShadowOpacity = 220,
  // Content drop shadow direction
  DropShadowDirection = 221,
  // Content drop shadow distance
  DropShadowDistance = 222,
  // Content drop shadow radius
  DropShadowRadius = 223,
}
/**
 * Enum for AnimaX dynamic properties related to the resource.
 */
export enum AnimaXResourcePropertyType {
  // Image asset directory name
  ImageDirName = 1,
  // Image asset file name
  ImageFileName = 2,
  // Image asset width
  ImageWidth = 3,
  // Image asset height
  ImageHeight = 4,
  // Font asset family
  FontFamily = 101,
  // Font asset style
  FontStyle = 102,
  // Font asset ascent
  FontAscent = 103,
  // Font asset path
  FontPath = 104,
  // Video asset directory name
  VideoDirName = 201,
  // Video asset file name
  VideoFileName = 202,
  // Video asset width
  VideoWidth = 203,
  // Video asset height
  VideoHeight = 204,
}

/** Basic animation event parameters */
export interface AnimaXParam {
  /** Unique animation ID */
  animationID: string;
  /** Current frame */
  current: number;
  /** Total frame count */
  total: number;
  /** Current loop index */
  loopIndex: number;
}
/** FPS event parameters */
export interface AnimaXFpsParam extends AnimaXParam {
  /** Current frames per second */
  fps: number;
  /** Maximum frame drop rate */
  max_drop_rate: number;
}
/** Tap layers event parameters */
export interface AnimaXTapParam extends AnimaXParam {
  /** List of tapped layers */
  layerList: string[];
}
/** Error or warning event parameters */
export interface AnimaXErrorOrWarningParam {
  /** Error or warning code */
  code: number;
  /** Data or description */
  data: string;
}

/** Supported event names */
export type AnimaXEventName =
  | 'ready'
  | 'completion'
  | 'start'
  | 'repeat'
  | 'cancel'
  | 'update'
  | 'error'
  | 'warning'
  | 'fps'
  | 'firstframe'
  | 'taplayers'
  | 'compositionready';

/** Generic custom event type */
export type AnimaXEvent<T> = CustomEvent<T>;
export type AnimaXReadyEvent = AnimaXEvent<AnimaXParam>;
export type AnimaXCompletionEvent = AnimaXEvent<AnimaXParam>;
export type AnimaXStartEvent = AnimaXEvent<AnimaXParam>;
export type AnimaXRepeatEvent = AnimaXEvent<AnimaXParam>;
export type AnimaXCancelEvent = AnimaXEvent<AnimaXParam>;
export type AnimaXUpdateEvent = AnimaXEvent<AnimaXParam>;
export type AnimaXErrorEvent = AnimaXEvent<AnimaXErrorOrWarningParam>;
export type AnimaXWarningEvent = AnimaXEvent<AnimaXErrorOrWarningParam>;
export type AnimaXFpsEvent = AnimaXEvent<AnimaXFpsParam>;
export type AnimaXFirstFrameEvent = AnimaXEvent<AnimaXParam>;
export type AnimaXTapLayersEvent = AnimaXEvent<AnimaXTapParam>;
export type AnimaXCompositionReadyEvent = AnimaXEvent<AnimaXParam>;

/**
 * Interface representing a 2D point with x and y coordinates.
 */
export interface AnimaXPoint {
  /** The x-coordinate of the point. */
  x: number;
  /** The y-coordinate of the point. */
  y: number;
}

/**
 * Interface for the parameters of a value, including optional types and a frame index. It is recommend to use createValueParam util to create ValueParam.
 */
export interface AnimaXValueParam {
  /** String value for text-based properties. */
  stringValue?: string;
  /** X-coordinate for point-based properties. Used with pointY together. */
  pointX?: number;
  /** Y-coordinate for point-based properties. Used with pointY together. */
  pointY?: number;
  /** Numeric value for double-precision properties. */
  doubleValue?: number;
  /** Boolean value for toggle properties. */
  boolValue?: boolean;
  /** The frame index for the value update. */
  frameIndex: number;
}

/**
 * Callback function for property updates.
 */
export type AnimaXPropertyCallback = (
  success: boolean,
  errorType: number
) => void;

export interface AnimaXViewProps {
  /** Backend to use for rendering. Default is 'webgl'.*/
  backend?: 'webgl' | 'webgpu';
  /** Animation JSON resource URL (supports http/https). */
  src?: string;
  /** Animation JSON resource path with placeholder ("%s"). Used with src-polyfill. */
  'src-format'?: string;
  /** Image resource mapping for animation (ID to URL). Used with src-format. */
  'src-polyfill'?: Record<string, string> | string;
  /** Animation resource content as a JSON string. */
  json?: string;
  /**
   * Width of the animation view, in pixels.
   * This value sets the underlying canvas's  attribute (pixel resolution),
   * but does not affect the CSS layout size of the canvas.
   */
  width?: number;
  /**
   * Height of the animation view, in pixels.
   * This value sets the underlying canvas's  attribute (pixel resolution),
   * but does not affect the CSS layout size of the canvas.
   */
  height?: number;
  /** Animation playback speed (1 = normal, 0.5 = half speed). */
  speed?: number;
  /** Whether to play automatically on load. */
  autoplay?: boolean;
  /** Frame to start playback from. */
  'start-frame'?: number;
  /** Frame to end playback at (-1 = last frame). */
  'end-frame'?: number;
  /** Whether to reverse animation during repeat playback. */
  'auto-reverse'?: boolean;
  /** Animation progress (range: 0 to 1). */
  progress?: number;
  /** Whether to loop animation. */
  loop?: boolean;
  /** Number of times to loop (0 = infinite). Do not use with loop. */
  'loop-count'?: number;
  /** How to fit the animation in the view ("contain", "cover", etc.). */
  objectfit?: 'contain' | 'cover' | 'center' | 'fill' | 'scale-down';
  /** How to align the animation in the view ("center", "left", etc.). */
  'object-position'?:
    | 'center'
    | 'left'
    | 'right'
    | 'top'
    | 'bottom'
    | 'top-left'
    | 'top-right'
    | 'bottom-left'
    | 'bottom-right';
  /** Interval for FPS event callback in milliseconds. Used with onFps. */
  'fps-event-interval'?: number;
  /** Maximum frame rate for rendering (recommended ≤ 120). */
  'max-frame-rate'?: number;
  /** Whether to use dynamic property features (requires manual play after ready). */
  'dynamic-resource'?: boolean;
  /** Whether to keep the last frame visible after playback ends. */
  keeplastframe?: boolean;
}

export interface AnimaXViewController {
  /** Start playing the animation from the beginning. */
  play(): void;
  /** Resume animation playback. */
  resume(): void;
  /** Pause animation playback. */
  pause(): void;
  /** Stop animation playback. */
  stop(): void;
  /**
   * Seek to a specific frame.
   * @param frame The target frame to seek to.
   */
  seek(frame: number): void;
  /**
   * Get the duration of the animation in milliseconds.
   * @returns The duration in ms.
   */
  getDuration(): number;
  /**
   * Subscribe to updates for a specific frame.
   * @param frame The frame number to subscribe to.
   */
  subscribeUpdateEvent(frame: number): void;
  /**
   * Unsubscribe from updates for a specific frame.
   * @param frame The frame number to unsubscribe from.
   */
  unsubscribeUpdateEvent(frame: number): void;
  /**
   * Subscribe to updates for multiple frames.
   * @param frames Array of frame numbers to subscribe to.
   */
  subscribeUpdateEvents(frames: number[]): void;
  /**
   * Unsubscribe from updates for multiple frames.
   * @param frames Array of frame numbers to unsubscribe from.
   */
  unsubscribeUpdateEvents(frames: number[]): void;
  /**
   * Get the current frame number.
   * @returns The current frame number.
   */
  getCurrentFrame(): number;
  /**
   * Check if the animation is currently playing.
   * @returns True if animating, false otherwise.
   */
  isAnimating(): boolean;
  /**
   * Play animation between specific frames.
   * @param startFrame The starting frame number.
   * @param endFrame The ending frame number.
   */
  playSegment(startFrame: number, endFrame: number): void;
  /**
   * Set a property of a resource.
   * @param resource_type The type of resource property to set.
   * @param resource_id The ID of the resource.
   * @param value The value to set for the property.
   * @param callback Callback function for asynchronous update.
   */
  setResourceProperty(
    resource_type: AnimaXResourcePropertyType,
    resource_id: string,
    value: AnimaXValueParam,
    callback?: AnimaXPropertyCallback
  ): void;
  /**
   * Update a property of a layer.
   * @param layer_type The type of layer property to update.
   * @param layer_name The name of the layer.
   * @param value The value to set for the property.
   * @param callback Callback function for asynchronous update.
   */
  updateLayerProperty(
    layer_type: AnimaXLayerPropertyType,
    layer_name: string,
    value: AnimaXValueParam,
    callback?: AnimaXPropertyCallback
  ): void;
  /**
   * Update the text size of a target text layer.
   * @param layerName The name of the layer as defined in After Effects (.aep file).
   * @param textSize The new text size value.
   * @param targetFrame The target keyframe index for update. Default is -1 (all frames).
   * @param callback Optional callback for asynchronous updates.
   */
  updateTextSizeByLayerName(
    layerName: string,
    textSize: number,
    targetFrame?: number,
    callback?: AnimaXPropertyCallback
  ): void;
  /**
   * Update the text color of a target text layer.
   * @param layerName The name of the layer as defined in After Effects (.aep file).
   * @param textColor The new text color (hex string).
   * @param targetFrame The target keyframe index for update. Default is -1 (all frames).
   * @param callback Optional callback for asynchronous updates.
   * @returns A boolean value that returns false when the input color is invalid.
   */
  updateTextColorByLayerName(
    layerName: string,
    textColor: string,
    targetFrame?: number,
    callback?: AnimaXPropertyCallback
  ): boolean;
  /**
   * Update the text content of a target text layer.
   * @param layerName The name of the layer as defined in After Effects (.aep file).
   * @param newText The new text content to set for the text layer.
   * @param targetFrame The target keyframe index for update. Default is -1 (all frames).
   * @param callback Optional callback for asynchronous updates.
   */
  updateTextByLayerName(
    layerName: string,
    newText: string,
    targetFrame?: number,
    callback?: AnimaXPropertyCallback
  ): void;
  /**
   * Update the image resource with a new image URL.
   * @param imageId The ID of the image as defined in the "assets" - "id" item.
   * @param newImageUrl The new image URL for the image resource.
   */
  updateImageById(imageId: string, newImageUrl: string): void;
  /**
   * Update the video resource with a new video URL.
   * @param videoId The ID of the video as defined in the "videos" - "id" item.
   * @param newVideoUrl The new video URL for the video resource.
   */
  updateVideoById(videoId: string, newVideoUrl: string): void;
  /**
   * Update the font asset with a new font path.
   * @param fontName The name of the font as defined in the "fonts" - "list" - "fName" item.
   * @param newFontPath The new font path URL for the font resource. Ensure "origin" is set to 3 in the .json file.
   */
  updateFontByName(fontName: string, newFontPath: string): void;
}
