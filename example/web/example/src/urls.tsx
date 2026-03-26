import {
  AnimaXLayerPropertyType,
  AnimaXResourcePropertyType,
  AnimaXValueParam,
  createAnimaXValueParam,
} from '@byted-lynx/animax';

export interface AnimaXPropertyKeyValue {
  type: AnimaXLayerPropertyType | AnimaXResourcePropertyType;
  key?: string;
  value: AnimaXValueParam;
}

export interface AnimaPropertyButton {
  label: string;
  is_resource?: boolean;
  data: AnimaXPropertyKeyValue | AnimaXPropertyKeyValue[];
}

export interface AnimaPageUrl {
  name: string;
  url: string;
  propertyButtons?: AnimaPropertyButton[];
}

export const urls: AnimaPageUrl[] = [
  {
    name: 'Smile Lottie',
    url:
      'https://lf3-static.bytednsdoc.com/obj/eden-cn/aulauvj-pvW/ljhwZthlaukjlkulzlp/lottie/smile-lottie.json',
  },
  {
    name: 'Preview Data',
    url:
      'https://lf3-static.bytednsdoc.com/obj/eden-cn/pxvhpt/preview/preview/data.json',
  },
  {
    name: 'Coin Lottie',
    url:
      'https://lf-dy-sourcecdn-tos.bytegecko.com/obj/byte-gurd-source/eden/react_lynx/aweme_lite/aweme_lite_react_lynx_lottie_new/resource/lottie/data.lottie.c5117e3a.json',
  },
  {
    name: 'Mall Lottie',
    url:
      'https://lf3-static.bytednsdoc.com/obj/eden-cn/ljylttvjl_lmp/ljhwZthlaukjlkulzlp/mall/build/1718175304255_data_0612.json',
  },
  {
    name: 'Image With Data URL Lottie',
    url:
      'https://lf3-static.bytednsdoc.com/obj/eden-cn/tzhd_ehtpuh/ljhwZthlaukjlkulzlp/LottieCases/ec_redpack_with_image_data_url.json',
  },
  {
    name: 'Treasure Box Lottie',
    url:
      'https://lf3-static.bytednsdoc.com/obj/eden-cn/3011eh7uptnuhog/wallet-home/lotties/treasure-box-v3/treasure.lottie.json',
  },
  {
    name: 'Mall 11 Lottie',
    url:
      'https://lf3-static.bytednsdoc.com/obj/eden-cn/huhfbe/data10.2829.json',
  },
  {
    name: 'Text Layout Lottie',
    url:
      'https://lf3-static.bytednsdoc.com/obj/eden-cn/tzhd_ehtpuh/ljhwZthlaukjlkulzlp/LottieCases/text_layout_250611.json',
  },
  {
    name: 'Simple Gradient Lottie',
    url:
      'https://lf3-static.bytednsdoc.com/obj/eden-cn/tzhd_ehtpuh/ljhwZthlaukjlkulzlp/LottieCases/SimpleGradient.json',
  },
  {
    name: 'Fish Package Lottie',
    url:
      'https://lf0-fast-deliver-inner.bytedance.net/obj/eden-internal/zlzhj_azla_ehtpuh/ljhwZthlaukjlkulzlp/business/UGTextProperty/data.json',
  },
  {
    name: 'Camera Composition Lottie',
    url:
      'https://lf3-static.bytednsdoc.com/obj/eden-cn/tzhd_ehtpuh/ljhwZthlaukjlkulzlp/LottieCases/CameraComposition.json',
  },
  {
    name: 'Metal Lottie',
    url:
      'https://sf-teko-source.tiktokcdn.com/obj/tiktok-teko-source-sg/tiktok/content/fe/gec_content_creator_growth_level/resource/lotties/level-6.lottie.54c26892.json',
  },
  {
    name: 'Dynamic Resource Text',
    url:
      'https://lf3-static.bytednsdoc.com/obj/eden-cn/rjpyeh7vhpinuvfph/lottie/text/StrokeThenFill.json',
    propertyButtons: [
      {
        label: 'Change Text',
        data: [
          {
            type: AnimaXLayerPropertyType.TextValue,
            key: 'strokethenfill',
            value: createAnimaXValueParam(
              'Hello 🔥🔥 こんにちは Привет 🌍🚀 !@#$% é ê π ∞ 123 MIXED Case 👨‍👩‍👧‍👦🇨🇳🇺🇸 Lorem ipsum ñ ç å 😀😂🥰 ∑∫√ العربية'
            ),
          },
          {
            type: AnimaXLayerPropertyType.Visibility,
            key: 'fillthenstroke',
            value: createAnimaXValueParam(false),
          },
        ],
      },
    ],
  },
  {
    name: 'Dynamic Layer Property',
    url:
      'https://lf0-fast-deliver-inner.bytedance.net/obj/eden-internal/rjpyeh7vhpinuvfph/animax/2025oncall/0728/dp2/data.json',
    propertyButtons: [
      {
        label: 'Show Layer',
        data: {
          type: AnimaXLayerPropertyType.Visibility,
          value: createAnimaXValueParam(1),
        },
      },
      {
        label: 'Hide Layer',
        data: {
          type: AnimaXLayerPropertyType.Visibility,
          value: createAnimaXValueParam(0),
        },
      },
      {
        label: 'Opacity 50%',
        data: {
          type: AnimaXLayerPropertyType.TransformOpacity,
          value: createAnimaXValueParam(50),
        },
      },
      {
        label: 'Opacity 100%',
        data: {
          type: AnimaXLayerPropertyType.TransformOpacity,
          value: createAnimaXValueParam(100),
        },
      },
      {
        label: 'Anchor 25x25',
        data: {
          type: AnimaXLayerPropertyType.TransformAnchor,
          value: createAnimaXValueParam({ x: 25, y: 25 }),
        },
      },
      {
        label: 'Position 50x50',
        data: {
          type: AnimaXLayerPropertyType.TransformPosition,
          value: createAnimaXValueParam({ x: 50, y: 50 }),
        },
      },
      {
        label: 'Scale 150%',
        data: {
          type: AnimaXLayerPropertyType.TransformScale,
          value: createAnimaXValueParam({ x: 1.5, y: 1.5 }),
        },
      },
      {
        label: 'Scale 100%',
        data: {
          type: AnimaXLayerPropertyType.TransformScale,
          value: createAnimaXValueParam({ x: 1, y: 1 }),
        },
      },
      {
        label: 'Rotate 45°',
        data: {
          type: AnimaXLayerPropertyType.TransformRotation,
          value: createAnimaXValueParam(45),
        },
      },
      {
        label: 'Reset Rotation',
        data: {
          type: AnimaXLayerPropertyType.TransformRotation,
          value: createAnimaXValueParam(0),
        },
      },
      {
        label: 'Skew 15°',
        data: {
          type: AnimaXLayerPropertyType.TransformSkew,
          value: createAnimaXValueParam(15),
        },
      },
      {
        label: 'Reset Skew',
        data: {
          type: AnimaXLayerPropertyType.TransformSkew,
          value: createAnimaXValueParam(0),
        },
      },
      {
        label: 'Skew Angle 30°',
        data: {
          type: AnimaXLayerPropertyType.TransformSkewAngle,
          value: createAnimaXValueParam(30),
        },
      },
      {
        label: 'Reset Skew Angle',
        data: {
          type: AnimaXLayerPropertyType.TransformSkewAngle,
          value: createAnimaXValueParam(0),
        },
      },
      {
        label: 'Start Opacity 80%',
        data: {
          type: AnimaXLayerPropertyType.TransformStartOpacity,
          value: createAnimaXValueParam(80),
        },
      },
      {
        label: 'Start Opacity 100%',
        data: {
          type: AnimaXLayerPropertyType.TransformStartOpacity,
          value: createAnimaXValueParam(100),
        },
      },
      {
        label: 'End Opacity 20%',
        data: {
          type: AnimaXLayerPropertyType.TransformEndOpacity,
          value: createAnimaXValueParam(20),
        },
      },
      {
        label: 'End Opacity 100%',
        data: {
          type: AnimaXLayerPropertyType.TransformEndOpacity,
          value: createAnimaXValueParam(100),
        },
      },
      {
        label: 'Rotate X 90°',
        data: {
          type: AnimaXLayerPropertyType.TransformRotationX,
          value: createAnimaXValueParam(90),
        },
      },
      {
        label: 'Reset Rotate X',
        data: {
          type: AnimaXLayerPropertyType.TransformRotationX,
          value: createAnimaXValueParam(0),
        },
      },
      {
        label: 'Rotate Y 45°',
        data: {
          type: AnimaXLayerPropertyType.TransformRotationY,
          value: createAnimaXValueParam(45),
        },
      },
      {
        label: 'Reset Rotate Y',
        data: {
          type: AnimaXLayerPropertyType.TransformRotationY,
          value: createAnimaXValueParam(0),
        },
      },
      {
        label: 'Rotate Z 180°',
        data: {
          type: AnimaXLayerPropertyType.TransformRotationZ,
          value: createAnimaXValueParam(180),
        },
      },
      {
        label: 'Reset Rotate Z',
        data: {
          type: AnimaXLayerPropertyType.TransformRotationZ,
          value: createAnimaXValueParam(0),
        },
      },
      // Text properties
      {
        label: 'Set Text "UPDATED"',
        data: {
          type: AnimaXLayerPropertyType.TextValue,
          value: createAnimaXValueParam('UPDATED'),
        },
      },
      {
        label: 'Set Text "Hello"',
        data: {
          type: AnimaXLayerPropertyType.TextValue,
          value: createAnimaXValueParam('Hello'),
        },
      },
      {
        label: 'Text Size 24px',
        data: {
          type: AnimaXLayerPropertyType.TextSize,
          value: createAnimaXValueParam(24),
        },
      },
      {
        label: 'Text Size 36px',
        data: {
          type: AnimaXLayerPropertyType.TextSize,
          value: createAnimaXValueParam(36),
        },
      },
      {
        label: 'Text Size 72px',
        data: {
          type: AnimaXLayerPropertyType.TextSize,
          value: createAnimaXValueParam(72),
        },
      },
      {
        label: 'Text Red',
        data: {
          type: AnimaXLayerPropertyType.TextColor,
          value: createAnimaXValueParam('#ff0000'),
        },
      },
      {
        label: 'Text Blue',
        data: {
          type: AnimaXLayerPropertyType.TextColor,
          value: createAnimaXValueParam('#0000ff'),
        },
      },
      {
        label: 'Text Green',
        data: {
          type: AnimaXLayerPropertyType.TextColor,
          value: createAnimaXValueParam('#00ff00'),
        },
      },
      {
        label: 'Text Tracking 5',
        data: {
          type: AnimaXLayerPropertyType.TextTracking,
          value: createAnimaXValueParam(5),
        },
      },
      {
        label: 'Text Tracking 0',
        data: {
          type: AnimaXLayerPropertyType.TextTracking,
          value: createAnimaXValueParam(0),
        },
      },
      // Color and visual properties
      {
        label: 'Fill Green',
        data: {
          type: AnimaXLayerPropertyType.Color,
          value: createAnimaXValueParam('#00ff00'),
        },
      },
      {
        label: 'Fill Blue',
        data: {
          type: AnimaXLayerPropertyType.Color,
          value: createAnimaXValueParam('#0000ff'),
        },
      },
      {
        label: 'Fill Red',
        data: {
          type: AnimaXLayerPropertyType.Color,
          value: createAnimaXValueParam('#ff0000'),
        },
      },
      {
        label: 'Color Filter Green',
        data: {
          type: AnimaXLayerPropertyType.ColorFilter,
          value: createAnimaXValueParam('#8000ff00'),
        },
      },
      {
        label: 'Color Filter Red',
        data: {
          type: AnimaXLayerPropertyType.ColorFilter,
          value: createAnimaXValueParam('#80ff0000'),
        },
      },
      {
        label: 'Stroke Purple',
        data: {
          type: AnimaXLayerPropertyType.StrokeColor,
          value: createAnimaXValueParam('#ff00ff'),
        },
      },
      {
        label: 'Stroke Blue',
        data: {
          type: AnimaXLayerPropertyType.StrokeColor,
          value: createAnimaXValueParam('#0080ff'),
        },
      },
      {
        label: 'Stroke Orange',
        data: {
          type: AnimaXLayerPropertyType.StrokeColor,
          value: createAnimaXValueParam('#ff8000'),
        },
      },
      {
        label: 'Stroke 5px',
        data: {
          type: AnimaXLayerPropertyType.StrokeWidth,
          value: createAnimaXValueParam(5),
        },
      },
      {
        label: 'Stroke 8px',
        data: {
          type: AnimaXLayerPropertyType.StrokeWidth,
          value: createAnimaXValueParam(8),
        },
      },
      {
        label: 'Stroke 2px',
        data: {
          type: AnimaXLayerPropertyType.StrokeWidth,
          value: createAnimaXValueParam(2),
        },
      },
      {
        label: 'Content Opacity 75%',
        data: {
          type: AnimaXLayerPropertyType.Opacity,
          value: createAnimaXValueParam(75),
        },
      },
      {
        label: 'Content Opacity 100%',
        data: {
          type: AnimaXLayerPropertyType.Opacity,
          value: createAnimaXValueParam(100),
        },
      },
      {
        label: 'Blur 10px',
        data: {
          type: AnimaXLayerPropertyType.BlurRadius,
          value: createAnimaXValueParam(10),
        },
      },
      {
        label: 'Blur Off',
        data: {
          type: AnimaXLayerPropertyType.BlurRadius,
          value: createAnimaXValueParam(0),
        },
      },
      // Shape properties
      {
        label: 'Ellipse 120x120',
        data: {
          type: AnimaXLayerPropertyType.EllipseSize,
          value: createAnimaXValueParam({ x: 120, y: 120 }),
        },
      },
      {
        label: 'Ellipse 80x80',
        data: {
          type: AnimaXLayerPropertyType.EllipseSize,
          value: createAnimaXValueParam({ x: 80, y: 80 }),
        },
      },
      {
        label: 'Rectangle 150x100',
        data: {
          type: AnimaXLayerPropertyType.RectangleSize,
          value: createAnimaXValueParam({ x: 150, y: 100 }),
        },
      },
      {
        label: 'Rectangle 100x100',
        data: {
          type: AnimaXLayerPropertyType.RectangleSize,
          value: createAnimaXValueParam({ x: 100, y: 100 }),
        },
      },
      {
        label: 'Corner 20px',
        data: {
          type: AnimaXLayerPropertyType.CornerRadius,
          value: createAnimaXValueParam(20),
        },
      },
      {
        label: 'Corner 25px',
        data: {
          type: AnimaXLayerPropertyType.CornerRadius,
          value: createAnimaXValueParam(25),
        },
      },
      {
        label: 'Corner 0px',
        data: {
          type: AnimaXLayerPropertyType.CornerRadius,
          value: createAnimaXValueParam(0),
        },
      },
      {
        label: 'Content Position 60x60',
        data: {
          type: AnimaXLayerPropertyType.Position,
          value: createAnimaXValueParam({ x: 60, y: 60 }),
        },
      },
      {
        label: 'Content Position 0x0',
        data: {
          type: AnimaXLayerPropertyType.Position,
          value: createAnimaXValueParam({ x: 0, y: 0 }),
        },
      },
      // Repeater properties
      {
        label: 'Repeater 4 Copies',
        data: {
          type: AnimaXLayerPropertyType.RepeaterCopies,
          value: createAnimaXValueParam(4),
        },
      },
      {
        label: 'Repeater 2 Copies',
        data: {
          type: AnimaXLayerPropertyType.RepeaterCopies,
          value: createAnimaXValueParam(2),
        },
      },
      {
        label: 'Repeater 1 Copy',
        data: {
          type: AnimaXLayerPropertyType.RepeaterCopies,
          value: createAnimaXValueParam(1),
        },
      },
      {
        label: 'Repeater Offset 50x50',
        data: {
          type: AnimaXLayerPropertyType.RepeaterOffset,
          value: createAnimaXValueParam({ x: 50, y: 50 }),
        },
      },
      {
        label: 'Repeater Offset 20x20',
        data: {
          type: AnimaXLayerPropertyType.RepeaterOffset,
          value: createAnimaXValueParam({ x: 20, y: 20 }),
        },
      },
      // PolyStar properties
      {
        label: 'PolyStar 6 Points',
        data: {
          type: AnimaXLayerPropertyType.PolystarPoints,
          value: createAnimaXValueParam(6),
        },
      },
      {
        label: 'PolyStar 5 Points',
        data: {
          type: AnimaXLayerPropertyType.PolystarPoints,
          value: createAnimaXValueParam(5),
        },
      },
      {
        label: 'PolyStar 8 Points',
        data: {
          type: AnimaXLayerPropertyType.PolystarPoints,
          value: createAnimaXValueParam(8),
        },
      },
      {
        label: 'PolyStar Rotation 60°',
        data: {
          type: AnimaXLayerPropertyType.PolystarRotation,
          value: createAnimaXValueParam(60),
        },
      },
      {
        label: 'PolyStar Rotation 0°',
        data: {
          type: AnimaXLayerPropertyType.PolystarRotation,
          value: createAnimaXValueParam(0),
        },
      },
      {
        label: 'PolyStar Inner Radius 30px',
        data: {
          type: AnimaXLayerPropertyType.PolystarInnerRadius,
          value: createAnimaXValueParam(30),
        },
      },
      {
        label: 'PolyStar Inner Radius 20px',
        data: {
          type: AnimaXLayerPropertyType.PolystarInnerRadius,
          value: createAnimaXValueParam(20),
        },
      },
      {
        label: 'PolyStar Outer Radius 60px',
        data: {
          type: AnimaXLayerPropertyType.PolystarOuterRadius,
          value: createAnimaXValueParam(60),
        },
      },
      {
        label: 'PolyStar Outer Radius 40px',
        data: {
          type: AnimaXLayerPropertyType.PolystarOuterRadius,
          value: createAnimaXValueParam(40),
        },
      },
      {
        label: 'PolyStar Inner Rounded 10px',
        data: {
          type: AnimaXLayerPropertyType.PolystarInnerRounded,
          value: createAnimaXValueParam(10),
        },
      },
      {
        label: 'PolyStar Inner Rounded 0px',
        data: {
          type: AnimaXLayerPropertyType.PolystarInnerRounded,
          value: createAnimaXValueParam(0),
        },
      },
      {
        label: 'PolyStar Outer Rounded 15px',
        data: {
          type: AnimaXLayerPropertyType.PolystarOuterRounded,
          value: createAnimaXValueParam(15),
        },
      },
      {
        label: 'PolyStar Outer Rounded 0px',
        data: {
          type: AnimaXLayerPropertyType.PolystarOuterRounded,
          value: createAnimaXValueParam(0),
        },
      },
      // Drop Shadow properties
      {
        label: 'Shadow Color Black',
        data: {
          type: AnimaXLayerPropertyType.DropShadowColor,
          value: createAnimaXValueParam('#000000'),
        },
      },
      {
        label: 'Shadow Color Red',
        data: {
          type: AnimaXLayerPropertyType.DropShadowColor,
          value: createAnimaXValueParam('#ff0000'),
        },
      },
      {
        label: 'Shadow Opacity 80%',
        data: {
          type: AnimaXLayerPropertyType.DropShadowOpacity,
          value: createAnimaXValueParam(80),
        },
      },
      {
        label: 'Shadow Opacity 50%',
        data: {
          type: AnimaXLayerPropertyType.DropShadowOpacity,
          value: createAnimaXValueParam(50),
        },
      },
      {
        label: 'Shadow Direction 135°',
        data: {
          type: AnimaXLayerPropertyType.DropShadowDirection,
          value: createAnimaXValueParam(135),
        },
      },
      {
        label: 'Shadow Direction 45°',
        data: {
          type: AnimaXLayerPropertyType.DropShadowDirection,
          value: createAnimaXValueParam(45),
        },
      },
      {
        label: 'Shadow Distance 20px',
        data: {
          type: AnimaXLayerPropertyType.DropShadowDistance,
          value: createAnimaXValueParam(20),
        },
      },
      {
        label: 'Shadow Distance 10px',
        data: {
          type: AnimaXLayerPropertyType.DropShadowDistance,
          value: createAnimaXValueParam(10),
        },
      },
      {
        label: 'Shadow Blur 10px',
        data: {
          type: AnimaXLayerPropertyType.DropShadowRadius,
          value: createAnimaXValueParam(10),
        },
      },
      {
        label: 'Shadow Blur 5px',
        data: {
          type: AnimaXLayerPropertyType.DropShadowRadius,
          value: createAnimaXValueParam(5),
        },
      },
    ],
  },
  {
    name: 'Dynamic Resource Properties',
    url:
      'https://lf0-fast-deliver-inner.bytedance.net/obj/eden-internal/rjpyeh7vhpinuvfph/animax/2025oncall/0728/dp3/data.json',
    propertyButtons: [
      // Image src
      {
        label: 'Update Image 0',
        is_resource: true,
        data: [
          {
            type: AnimaXResourcePropertyType.ImageDirName,
            key: 'image_0',
            value: createAnimaXValueParam(''),
          },
          {
            type: AnimaXResourcePropertyType.ImageFileName,
            key: 'image_0',
            value: createAnimaXValueParam(
              'https://lf0-fast-deliver-inner.bytedance.net/obj/eden-internal/rjpyeh7vhpinuvfph/animax/2025oncall/0728/xixixi/images/img_10.png'
            ),
          },
        ],
      },
      {
        label: 'Update Image 1',
        is_resource: true,
        data: [
          {
            type: AnimaXResourcePropertyType.ImageDirName,
            key: 'image_1',
            value: createAnimaXValueParam(''),
          },
          {
            type: AnimaXResourcePropertyType.ImageFileName,
            key: 'image_1',
            value: createAnimaXValueParam(
              'https://lf0-fast-deliver-inner.bytedance.net/obj/eden-internal/rjpyeh7vhpinuvfph/animax/2025oncall/0728/xixixi/images/img_5.png'
            ),
          },
        ],
      },
      // Font src
      {
        label: 'Set Sans',
        is_resource: true,
        data: {
          type: AnimaXResourcePropertyType.FontPath,
          key: 'STHeitiSC-Light',
          value: createAnimaXValueParam(
            'https://lf-sourcecdn-tos.bytegecko.com/obj/byte-gurd-source/11561/gecko/resource/ecom_font_sans/tca-871836108290-1.4.6/fonts/DouyinSansBold.ttf'
          ),
        },
      },
      {
        label: 'Set NumberABC',
        is_resource: true,
        data: {
          type: AnimaXResourcePropertyType.FontPath,
          key: 'STHeitiSC-Light',
          value: createAnimaXValueParam(
            'https://lf0-fast-deliver-inner.bytedance.net/obj/eden-internal/rjpyeh7vhpinuvfph/animax/2025oncall/0728/xixixi/fonts/DouyinNumberABC_otf.ttf'
          ),
        },
      },
      // Image width
      {
        label: 'Image_0 Width 200px',
        is_resource: true,
        data: {
          type: AnimaXResourcePropertyType.ImageWidth,
          key: 'image_0',
          value: createAnimaXValueParam(200),
        },
      },
      {
        label: 'Image_1 Width 150px',
        is_resource: true,
        data: {
          type: AnimaXResourcePropertyType.ImageWidth,
          key: 'image_1',
          value: createAnimaXValueParam(150),
        },
      },
      // Image height
      {
        label: 'Image_0 Height 200px',
        is_resource: true,
        data: {
          type: AnimaXResourcePropertyType.ImageHeight,
          key: 'image_0',
          value: createAnimaXValueParam(200),
        },
      },
      {
        label: 'Image_1 Height 150px',
        is_resource: true,
        data: {
          type: AnimaXResourcePropertyType.ImageHeight,
          key: 'image_1',
          value: createAnimaXValueParam(150),
        },
      },
      // Font family
      {
        label: 'Font Family Arial',
        is_resource: true,
        data: {
          type: AnimaXResourcePropertyType.FontFamily,
          key: 'STHeitiSC-Light',
          value: createAnimaXValueParam('Arial'),
        },
      },
      {
        label: 'Font Family Helvetica',
        is_resource: true,
        data: {
          type: AnimaXResourcePropertyType.FontFamily,
          key: 'STHeitiSC-Light',
          value: createAnimaXValueParam('Helvetica'),
        },
      },
      // Font style
      {
        label: 'Font Style Normal',
        is_resource: true,
        data: {
          type: AnimaXResourcePropertyType.FontStyle,
          key: 'STHeitiSC-Light',
          value: createAnimaXValueParam('normal'),
        },
      },
      {
        label: 'Font Style Italic',
        is_resource: true,
        data: {
          type: AnimaXResourcePropertyType.FontStyle,
          key: 'STHeitiSC-Light',
          value: createAnimaXValueParam('italic'),
        },
      },
      // Font ascent
      {
        label: 'Font Ascent 800',
        is_resource: true,
        data: {
          type: AnimaXResourcePropertyType.FontAscent,
          key: 'STHeitiSC-Light',
          value: createAnimaXValueParam(800),
        },
      },
      {
        label: 'Font Ascent 750',
        is_resource: true,
        data: {
          type: AnimaXResourcePropertyType.FontAscent,
          key: 'STHeitiSC-Light',
          value: createAnimaXValueParam(750),
        },
      },
    ],
  },
];
