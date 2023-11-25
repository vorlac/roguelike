#pragma once

#include <type_traits>

#include <glad/gl.h>

#include "core/numeric_types.hpp"
#include "sdl/defs.hpp"

namespace rl::gl {

    using glenum = rl::u32;
    using glbool_t = rl::u8;
    using glbitfield = rl::u32;
    using glvoid = void;
    using glbyte = rl::i8;
    using glshort = rl::i16;
    using glint = rl::i32;
    using glubyte = rl::u8;
    using glushort = rl::u16;
    using gluint = rl::u32;
    using glsizei = rl::i32;
    using glfloat = rl::f32;
    using glclampf = rl::f32;
    using gldouble = rl::f64;
    using glclampd = rl::f64;

    // OpenGL Boolean
    enum glbool : glbool_t {
        False = GL_FALSE,
        True = GL_TRUE,
    };

    // OpenGL Data Type
    enum gltype : glenum {
        SByte = GL_BYTE,
        UByte = GL_UNSIGNED_BYTE,
        SShort = GL_SHORT,
        UShort = GL_UNSIGNED_SHORT,
        SInt = GL_INT,
        UInt = GL_UNSIGNED_INT,
        Float = GL_FLOAT,
        Double = GL_DOUBLE,
    };

    // OpenGL Primitives Types
    enum glprimitive : glenum {
        Points = GL_POINTS,
        Lines = GL_LINES,
        LineLoop = GL_LINE_LOOP,
        LineStrip = GL_LINE_STRIP,
        Triangles = GL_TRIANGLES,
        TriangleStrip = GL_TRIANGLE_STRIP,
        TriangleFan = GL_TRIANGLE_FAN,
        Quads = GL_QUADS,
        // QuadStrip = GL_QUAD_STRIP,
        // Polygon = GL_POLYGON,
    };

    // OpenGL Vertex Arrays
    enum glvertarray : glenum {
        VertexArray = GL_VERTEX_ARRAY,
        // NormalArray = GL_NORMAL_ARRAY,
        // ColorArray = GL_COLOR_ARRAY,
        // IndexArray = GL_INDEX_ARRAY,
        // TextureCoordArray = GL_TEXTURE_COORD_ARRAY,
        // EdgeFlagArray = GL_EDGE_FLAG_ARRAY,
        // VertexArraySize = GL_VERTEX_ARRAY_SIZE,
        // VertexArrayType = GL_VERTEX_ARRAY_TYPE,
        // VertexArrayStride = GL_VERTEX_ARRAY_STRIDE,
        // NormalArrayType = GL_NORMAL_ARRAY_TYPE,
        // NormalArrayStride = GL_NORMAL_ARRAY_STRIDE,
        // ColorArraySize = GL_COLOR_ARRAY_SIZE,
        // ColorArrayType = GL_COLOR_ARRAY_TYPE,
        // ColorArrayStride = GL_COLOR_ARRAY_STRIDE,
        // IndexArrayType = GL_INDEX_ARRAY_TYPE,
        // IndexArrayStride = GL_INDEX_ARRAY_STRIDE,
        // TextureCoordArraySize = GL_TEXTURE_COORD_ARRAY_SIZE,
        // TextureCoordArrayType = GL_TEXTURE_COORD_ARRAY_TYPE,
        // TextureCoordArrayStride = GL_TEXTURE_COORD_ARRAY_STRIDE,
        // EdgeFlagArrayStride = GL_EDGE_FLAG_ARRAY_STRIDE,
        // VertexArrayPointer = GL_VERTEX_ARRAY_POINTER,
        // NormalArrayPointer = GL_NORMAL_ARRAY_POINTER,
        // ColorArrayPointer = GL_COLOR_ARRAY_POINTER,
        // IndexArrayPointer = GL_INDEX_ARRAY_POINTER,
        // TextureCoordArrayPointer = GL_TEXTURE_COORD_ARRAY_POINTER,
        // EdgeFlagArrayPointer = GL_EDGE_FLAG_ARRAY_POINTER,
        // V2F = GL_V2F,
        // V3F = GL_V3F,
        // C4UB_V2F = GL_C4UB_V2F,
        // C4UB_V3F = GL_C4UB_V3F,
        // C3F_V3F = GL_C3F_V3F,
        // N3F_V3F = GL_N3F_V3F,
        // C4F_N3F_V3F = GL_C4F_N3F_V3F,
        // T2F_V3F = GL_T2F_V3F,
        // T4F_V4F = GL_T4F_V4F,
        // T2F_C4UB_V3F = GL_T2F_C4UB_V3F,
        // T2F_C3F_V3F = GL_T2F_C3F_V3F,
        // T2F_N3F_V3F = GL_T2F_N3F_V3F,
        // T2F_C4F_N3F_V3F = GL_T2F_C4F_N3F_V3F,
        // T4F_C4F_N3F_V4F = GL_T4F_C4F_N3F_V4F,
    };

    // OpenGL Matrix Mode
    enum glmatrixmode : glenum {
        // MatrixMode = GL_MATRIX_MODE,
        // Modelview = GL_MODELVIEW,
        // Projection = GL_PROJECTION,
        Texture = GL_TEXTURE,
    };

    // OpenGL Points
    enum glpoints : glenum {
        // PointSmooth = GL_POINT_SMOOTH,
        PointSize = GL_POINT_SIZE,
        PointSizeGranularity = GL_POINT_SIZE_GRANULARITY,
        PointSizeRange = GL_POINT_SIZE_RANGE,
    };

    // OpenGL Lines
    enum gllines : glenum {
        LineSmooth = GL_LINE_SMOOTH,
        // LineStipple = GL_LINE_STIPPLE,
        // LineStipplePattern = GL_LINE_STIPPLE_PATTERN,
        // LineStippleRepeat = GL_LINE_STIPPLE_REPEAT,
        LineWidth = GL_LINE_WIDTH,
        LineWidthGranularity = GL_LINE_WIDTH_GRANULARITY,
        LineWidthRange = GL_LINE_WIDTH_RANGE,
    };

    // OpenGL Polygons
    enum glpolys : glenum {
        Point = GL_POINT,
        Line = GL_LINE,
        Fill = GL_FILL,
        CW = GL_CW,
        CCW = GL_CCW,
        Front = GL_FRONT,
        Back = GL_BACK,
        PolygonMode = GL_POLYGON_MODE,
        PolygonSmooth = GL_POLYGON_SMOOTH,
        // PolygonStipple = GL_POLYGON_STIPPLE,
        // EdgeFlag = GL_EDGE_FLAG,
        CullFace = GL_CULL_FACE,
        CullFaceMode = GL_CULL_FACE_MODE,
        FrontFace = GL_FRONT_FACE,
        PolygonOffsetFactor = GL_POLYGON_OFFSET_FACTOR,
        PolygonOffsetUnits = GL_POLYGON_OFFSET_UNITS,
        PolygonOffsetPoint = GL_POLYGON_OFFSET_POINT,
        PolygonOffsetLine = GL_POLYGON_OFFSET_LINE,
        PolygonOffsetFill = GL_POLYGON_OFFSET_FILL,
    };

    // OpenGL Display Lists
    // enum gldispaylist : glenum {
    //    Compile = GL_COMPILE,
    //    CompileAndExecute = GL_COMPILE_AND_EXECUTE,
    //    ListBase = GL_LIST_BASE,
    //    ListIndex = GL_LIST_INDEX,
    //    ListMode = GL_LIST_MODE,
    //};

    // OpenGL Depth buffer
    enum gldepthbuff : glenum {
        Never = GL_NEVER,
        Less = GL_LESS,
        Equal = GL_EQUAL,
        Lequal = GL_LEQUAL,
        Greater = GL_GREATER,
        Notequal = GL_NOTEQUAL,
        Gequal = GL_GEQUAL,
        Always = GL_ALWAYS,
        DepthTest = GL_DEPTH_TEST,
        DepthBits = GL_DEPTH_TEXTURE_MODE_ARB,
        DepthClearValue = GL_DEPTH_CLEAR_VALUE,
        DepthFunc = GL_DEPTH_FUNC,
        DepthRange = GL_DEPTH_RANGE,
        DepthWritemask = GL_DEPTH_WRITEMASK,
        DepthComponent = GL_DEPTH_COMPONENT,
    };

    // OpenGL Lighting
    // enum gllighting : glenum {
    //    Lighting = GL_LIGHTING,
    //    Light0 = GL_LIGHT0,
    //    Light1 = GL_LIGHT1,
    //    Light2 = GL_LIGHT2,
    //    Light3 = GL_LIGHT3,
    //    Light4 = GL_LIGHT4,
    //    Light5 = GL_LIGHT5,
    //    Light6 = GL_LIGHT6,
    //    Light7 = GL_LIGHT7,
    //    SpotExponent = GL_SPOT_EXPONENT,
    //    SpotCutoff = GL_SPOT_CUTOFF,
    //    ConstantAttenuation = GL_CONSTANT_ATTENUATION,
    //    LinearAttenuation = GL_LINEAR_ATTENUATION,
    //    QuadraticAttenuation = GL_QUADRATIC_ATTENUATION,
    //    Ambient = GL_AMBIENT,
    //    Diffuse = GL_DIFFUSE,
    //    Specular = GL_SPECULAR,
    //    Shininess = GL_SHININESS,
    //    Emission = GL_EMISSION,
    //    Position = GL_POSITION,
    //    SpotDirection = GL_SPOT_DIRECTION,
    //    AmbientAndDiffuse = GL_AMBIENT_AND_DIFFUSE,
    //    ColorIndexes = GL_COLOR_INDEXES,
    //    LightModelTwoSide = GL_LIGHT_MODEL_TWO_SIDE,
    //    LightModelLocalViewer = GL_LIGHT_MODEL_LOCAL_VIEWER,
    //    LightModelAmbient = GL_LIGHT_MODEL_AMBIENT,
    //    FrontAndBack = GL_FRONT_AND_BACK,
    //    ShadeModel = GL_SHADE_MODEL,
    //    Flat = GL_FLAT,
    //    Smooth = GL_SMOOTH,
    //    ColorMaterial = GL_COLOR_MATERIAL,
    //    ColorMaterialFace = GL_COLOR_MATERIAL_FACE,
    //    ColorMaterialParameter = GL_COLOR_MATERIAL_PARAMETER,
    //    Normalize = GL_NORMALIZE,
    //};

    // OpenGL user clipping planes
    // enum glclipping : glenum {
    //     ClipPlane0 = GL_CLIP_PLANE0,
    //     ClipPlane1 = GL_CLIP_PLANE1,
    //     ClipPlane2 = GL_CLIP_PLANE2,
    //     ClipPlane3 = GL_CLIP_PLANE3,
    //     ClipPlane4 = GL_CLIP_PLANE4,
    //     ClipPlane5 = GL_CLIP_PLANE5,
    // };

    // OpenGL Accumulation buffer
    // enum glaccumbuff : glenum {
    //     AccumRedBits = GL_ACCUM_RED_BITS,
    //     AccumGreenBits = GL_ACCUM_GREEN_BITS,
    //     AccumBlueBits = GL_ACCUM_BLUE_BITS,
    //     AccumAlphaBits = GL_ACCUM_ALPHA_BITS,
    //     AccumClearValue = GL_ACCUM_CLEAR_VALUE,
    //     Accum = GL_ACCUM,
    //     Add = GL_ADD,
    //     Load = GL_LOAD,
    //     Mult = GL_MULT,
    //     Return = GL_RETURN,
    // };

    // OpenGL Alpha testing
    // enum glalphatest : glenum {
    //    AlphaTest = GL_ALPHA_TEST,
    //    AlphaTestRef = GL_ALPHA_TEST_REF,
    //    AlphaTestFunc = GL_ALPHA_TEST_FUNC,
    //};

    // OpenGL Blending
    enum glblending : glenum {
        Blend = GL_BLEND,
        BlendSrc = GL_BLEND_SRC,
        BlendDst = GL_BLEND_DST,
        Zero = GL_ZERO,
        One = GL_ONE,
        SrcColor = GL_SRC_COLOR,
        OneMinusSrcColor = GL_ONE_MINUS_SRC_COLOR,
        SrcAlpha = GL_SRC_ALPHA,
        OneMinusSrcAlpha = GL_ONE_MINUS_SRC_ALPHA,
        DstAlpha = GL_DST_ALPHA,
        OneMinusDstAlpha = GL_ONE_MINUS_DST_ALPHA,
        DstColor = GL_DST_COLOR,
        OneMinusDstColor = GL_ONE_MINUS_DST_COLOR,
        SrcAlphaSaturate = GL_SRC_ALPHA_SATURATE,
    };

    // OpenGL Render Mode
    // enum glrendermode : glenum {
    //     Feedback = GL_FEEDBACK,
    //     Render = GL_RENDER,
    //     Select = GL_SELECT,
    // };

    // OpenGL Feedback
    // enum class glfeedback : glenum {
    //     2D = GL_2D,
    //     3D = GL_3D,
    //     3DColor = GL_3D_COLOR,
    //     3DColorTexture = GL_3D_COLOR_TEXTURE,
    //     4DColorTexture = GL_4D_COLOR_TEXTURE,
    //     PointToken = GL_POINT_TOKEN,
    //     LineToken = GL_LINE_TOKEN,
    //     LineResetToken = GL_LINE_RESET_TOKEN,
    //     PolygonToken = GL_POLYGON_TOKEN,
    //     BitmapToken = GL_BITMAP_TOKEN,
    //     DrawPixelToken = GL_DRAW_PIXEL_TOKEN,
    //     CopyPixelToken = GL_COPY_PIXEL_TOKEN,
    //     PassThroughToken = GL_PASS_THROUGH_TOKEN,
    //     FeedbackBufferPointer = GL_FEEDBACK_BUFFER_POINTER,
    //     FeedbackBufferSize = GL_FEEDBACK_BUFFER_SIZE,
    //     FeedbackBufferType = GL_FEEDBACK_BUFFER_TYPE,
    // };

    // OpenGL Selection
    // enum glselection : glenum {
    //    SelectionBufferPointer = GL_SELECTION_BUFFER_POINTER,
    //    SelectionBufferSize = GL_SELECTION_BUFFER_SIZE,
    //};

    // OpenGL Fog
    enum glfog : glenum {
        Fog = GL_FOG,
        //     FogMode = GL_FOG_MODE,
        //     FogDensity = GL_FOG_DENSITY,
        //     FogColor = GL_FOG_COLOR,
        //     FogIndex = GL_FOG_INDEX,
        //     FogStart = GL_FOG_START,
        //     FogEnd = GL_FOG_END,
        Linear = GL_LINEAR,
        //    Exp = GL_EXP,
        //    Exp2 = GL_EXP2,
    };

    // OpenGL Logic Ops
    enum glop : glenum {
        //     LogicOp = GL_LOGIC_OP,
        //     IndexLogicOp = GL_INDEX_LOGIC_OP,
        ColorLogicOp = GL_COLOR_LOGIC_OP,
        LogicOpMode = GL_LOGIC_OP_MODE,
        Clear = GL_CLEAR,
        Set = GL_SET,
        Copy = GL_COPY,
        CopyInverted = GL_COPY_INVERTED,
        Noop = GL_NOOP,
        Invert = GL_INVERT,
        And = GL_AND,
        Nand = GL_NAND,
        Or = GL_OR,
        Nor = GL_NOR,
        Xor = GL_XOR,
        Equiv = GL_EQUIV,
        AndReverse = GL_AND_REVERSE,
        AndInverted = GL_AND_INVERTED,
        OrReverse = GL_OR_REVERSE,
        OrInverted = GL_OR_INVERTED,
    };

    // OpenGL Stencil
    enum glstencil : glenum {
        //    StencilBits = GL_STENCIL_BITS,
        StencilTest = GL_STENCIL_TEST,
        StencilClearValue = GL_STENCIL_CLEAR_VALUE,
        StencilFunc = GL_STENCIL_FUNC,
        StencilValueMask = GL_STENCIL_VALUE_MASK,
        StencilFail = GL_STENCIL_FAIL,
        StencilPassDepthFail = GL_STENCIL_PASS_DEPTH_FAIL,
        StencilPassDepthPass = GL_STENCIL_PASS_DEPTH_PASS,
        StencilRef = GL_STENCIL_REF,
        StencilWritemask = GL_STENCIL_WRITEMASK,
        StencilIndex = GL_STENCIL_INDEX,
        Keep = GL_KEEP,
        Replace = GL_REPLACE,
        Incr = GL_INCR,
        Decr = GL_DECR,
    };

    // OpenGL Buffers, Pixel Drawing/Reading
    enum glpixelbuff : glenum {
        None = GL_NONE,
        Left = GL_LEFT,
        Right = GL_RIGHT,
        // Front = GL_FRONT,
        // Back = GL_BACK,
        // FrontAndBack = GL_FRONT_AND_BACK,
        FrontLeft = GL_FRONT_LEFT,
        FrontRight = GL_FRONT_RIGHT,
        BackLeft = GL_BACK_LEFT,
        BackRight = GL_BACK_RIGHT,
        //  Aux0 = GL_AUX0,
        //  Aux1 = GL_AUX1,
        //  Aux2 = GL_AUX2,
        //  Aux3 = GL_AUX3,
        //  ColorIndex = GL_COLOR_INDEX,
        Red = GL_RED,
        Green = GL_GREEN,
        Blue = GL_BLUE,
        Alpha = GL_ALPHA,
        //  LUMINANCE = GL_LUMINANCE,
        //  LuminanceAlpha = GL_LUMINANCE_ALPHA,
        //  AlphaBits = GL_ALPHA_BITS,
        //  RedBits = GL_RED_BITS,
        //  GreenBits = GL_GREEN_BITS,
        //  BlueBits = GL_BLUE_BITS,
        //  IndexBits = GL_INDEX_BITS,
        SubpixelBits = GL_SUBPIXEL_BITS,
        // AuxBuffers = GL_AUX_BUFFERS,
        ReadBuffer = GL_READ_BUFFER,
        DrawBuffer = GL_DRAW_BUFFER,
        DoubleBuffer = GL_DOUBLEBUFFER,
        Stereo = GL_STEREO,
        // Bitmap = GL_BITMAP,
        Color = GL_COLOR,
        Depth = GL_DEPTH,
        Stencil = GL_STENCIL,
        Dither = GL_DITHER,
        RGB = GL_RGB,
        RGBA = GL_RGBA,
    };

    // OpenGL Implementation limits
    enum glimpllimit : glenum {
        //  MaxListNesting = GL_MAX_LIST_NESTING,
        //  MaxEvalOrder = GL_MAX_EVAL_ORDER,
        //  MaxLights = GL_MAX_LIGHTS,
        //  MaxClipPlanes = GL_MAX_CLIP_PLANES,
        MaxTextureSize = GL_MAX_TEXTURE_SIZE,
        //  MaxPixelMapTable = GL_MAX_PIXEL_MAP_TABLE,
        //  MaxAttribStackDepth = GL_MAX_ATTRIB_STACK_DEPTH,
        //  MaxModelviewStackDepth = GL_MAX_MODELVIEW_STACK_DEPTH,
        //  MaxNameStackDepth = GL_MAX_NAME_STACK_DEPTH,
        //  MaxProjectionStackDepth = GL_MAX_PROJECTION_STACK_DEPTH,
        //  MaxTextureStackDepth = GL_MAX_TEXTURE_STACK_DEPTH,
        MaxViewportDims = GL_MAX_VIEWPORT_DIMS,
        // MaxClientAttribStackDepth = GL_MAX_CLIENT_ATTRIB_STACK_DEPTH,
    };

    // OpenGL Gets
    enum glget : glenum {
        //  AttribStackDepth = GL_ATTRIB_STACK_DEPTH,
        //  ClientAttribStackDepth = GL_CLIENT_ATTRIB_STACK_DEPTH,
        ColorClearValue = GL_COLOR_CLEAR_VALUE,
        ColorWritemask = GL_COLOR_WRITEMASK,
        // CurrentIndex = GL_CURRENT_INDEX,
        // CurrentColor = GL_CURRENT_COLOR,
        // CurrentNormal = GL_CURRENT_NORMAL,
        // CurrentRasterColor = GL_CURRENT_RASTER_COLOR,
        // CurrentRasterDistance = GL_CURRENT_RASTER_DISTANCE,
        // CurrentRasterIndex = GL_CURRENT_RASTER_INDEX,
        // CurrentRasterPosition = GL_CURRENT_RASTER_POSITION,
        // CurrentRasterTextureCoords = GL_CURRENT_RASTER_TEXTURE_COORDS,
        // CurrentRasterPositionValid = GL_CURRENT_RASTER_POSITION_VALID,
        // CurrentTextureCoords = GL_CURRENT_TEXTURE_COORDS,
        // IndexClearValue = GL_INDEX_CLEAR_VALUE,
        // IndexMode = GL_INDEX_MODE,
        // IndexWritemask = GL_INDEX_WRITEMASK,
        // ModelviewMatrix = GL_MODELVIEW_MATRIX,
        // ModelviewStackDepth = GL_MODELVIEW_STACK_DEPTH,
        // NameStackDepth = GL_NAME_STACK_DEPTH,
        // ProjectionMatrix = GL_PROJECTION_MATRIX,
        // ProjectionStackDepth = GL_PROJECTION_STACK_DEPTH,
        // RenderMode = GL_RENDER_MODE,
        // RgbaMode = GL_RGBA_MODE,
        // TextureMatrix = GL_TEXTURE_MATRIX,
        // TextureStackDepth = GL_TEXTURE_STACK_DEPTH,
        Viewport = GL_VIEWPORT,
    };

    // OpenGL Evaluators
    // enum gleval : glenum {
    //     AutoNormal = GL_AUTO_NORMAL,
    //     Map1Color4 = GL_MAP1_COLOR_4,
    //     Map1Index = GL_MAP1_INDEX,
    //     Map1Normal = GL_MAP1_NORMAL,
    //     Map1TextureCoord1 = GL_MAP1_TEXTURE_COORD_1,
    //     Map1TextureCoord2 = GL_MAP1_TEXTURE_COORD_2,
    //     Map1TextureCoord3 = GL_MAP1_TEXTURE_COORD_3,
    //     Map1TextureCoord4 = GL_MAP1_TEXTURE_COORD_4,
    //     Map1Vertex3 = GL_MAP1_VERTEX_3,
    //     Map1Vertex4 = GL_MAP1_VERTEX_4,
    //     Map2Color4 = GL_MAP2_COLOR_4,
    //     Map2Index = GL_MAP2_INDEX,
    //     Map2Normal = GL_MAP2_NORMAL,
    //     Map2TextureCoord1 = GL_MAP2_TEXTURE_COORD_1,
    //     Map2TextureCoord2 = GL_MAP2_TEXTURE_COORD_2,
    //     Map2TextureCoord3 = GL_MAP2_TEXTURE_COORD_3,
    //     Map2TextureCoord4 = GL_MAP2_TEXTURE_COORD_4,
    //     Map2Vertex3 = GL_MAP2_VERTEX_3,
    //     Map2Vertex4 = GL_MAP2_VERTEX_4,
    //     Map1GridDomain = GL_MAP1_GRID_DOMAIN,
    //     Map1GridSegments = GL_MAP1_GRID_SEGMENTS,
    //     Map2GridDomain = GL_MAP2_GRID_DOMAIN,
    //     Map2GridSegments = GL_MAP2_GRID_SEGMENTS,
    //     Coeff = GL_COEFF,
    //     Order = GL_ORDER,
    //     Domain = GL_DOMAIN,
    // };

    // OpenGL Hints
    enum glhint : glenum {
        //   PerspectiveCorrectionHint = GL_PERSPECTIVE_CORRECTION_HINT,
        //   PointSmoothHint = GL_POINT_SMOOTH_HINT,
        LineSmoothHint = GL_LINE_SMOOTH_HINT,
        PolygonSmoothHint = GL_POLYGON_SMOOTH_HINT,
        //  FogHint = GL_FOG_HINT,
        DontCare = GL_DONT_CARE,
        Fastest = GL_FASTEST,
        Nicest = GL_NICEST,
    };

    // OpenGL Scissor box
    enum glscissorbox : glenum {
        ScissorBox = GL_SCISSOR_BOX,
        ScissorTest = GL_SCISSOR_TEST,
    };

    // OpenGL Pixel Mode / Transfer
    enum glpixelmode : glenum {
        //    MapColor = GL_MAP_COLOR,
        //    MapStencil = GL_MAP_STENCIL,
        //    IndexShift = GL_INDEX_SHIFT,
        //    IndexOffset = GL_INDEX_OFFSET,
        //    RedScale = GL_RED_SCALE,
        //    RedBias = GL_RED_BIAS,
        //    GreenScale = GL_GREEN_SCALE,
        //    GreenBias = GL_GREEN_BIAS,
        //    BlueScale = GL_BLUE_SCALE,
        //    BlueBias = GL_BLUE_BIAS,
        //    AlphaScale = GL_ALPHA_SCALE,
        //    AlphaBias = GL_ALPHA_BIAS,
        //    DepthScale = GL_DEPTH_SCALE,
        //    DepthBias = GL_DEPTH_BIAS,
        //    PixelMapSToSSize = GL_PIXEL_MAP_S_TO_S_SIZE,
        //    PixelMapIToISize = GL_PIXEL_MAP_I_TO_I_SIZE,
        //    PixelMapIToRSize = GL_PIXEL_MAP_I_TO_R_SIZE,
        //    PixelMapIToGSize = GL_PIXEL_MAP_I_TO_G_SIZE,
        //    PixelMapIToBSize = GL_PIXEL_MAP_I_TO_B_SIZE,
        //    PixelMapIToASize = GL_PIXEL_MAP_I_TO_A_SIZE,
        //    PixelMapRToRSize = GL_PIXEL_MAP_R_TO_R_SIZE,
        //    PixelMapGToGSize = GL_PIXEL_MAP_G_TO_G_SIZE,
        //    PixelMapBToBSize = GL_PIXEL_MAP_B_TO_B_SIZE,
        //    PixelMapAToASize = GL_PIXEL_MAP_A_TO_A_SIZE,
        //    PixelMap_StoS = GL_PIXEL_MAP_S_TO_S,
        //    PixelMap_ItoI = GL_PIXEL_MAP_I_TO_I,
        //    PixelMap_ItoR = GL_PIXEL_MAP_I_TO_R,
        //    PixelMap_ItoG = GL_PIXEL_MAP_I_TO_G,
        //    PixelMap_ItoB = GL_PIXEL_MAP_I_TO_B,
        //    PixelMap_ItoA = GL_PIXEL_MAP_I_TO_A,
        //    PixelMap_RtoR = GL_PIXEL_MAP_R_TO_R,
        //    PixelMap_GtoG = GL_PIXEL_MAP_G_TO_G,
        //    PixelMap_BtoB = GL_PIXEL_MAP_B_TO_B,
        //    PixelMap_AtoA = GL_PIXEL_MAP_A_TO_A,
        PackAlignment = GL_PACK_ALIGNMENT,
        PackLsbFirst = GL_PACK_LSB_FIRST,
        PackRowLength = GL_PACK_ROW_LENGTH,
        PackSkipPixels = GL_PACK_SKIP_PIXELS,
        PackSkipRows = GL_PACK_SKIP_ROWS,
        PackSwapBytes = GL_PACK_SWAP_BYTES,
        UnpackAlignment = GL_UNPACK_ALIGNMENT,
        UnpackLsbFirst = GL_UNPACK_LSB_FIRST,
        UnpackRowLength = GL_UNPACK_ROW_LENGTH,
        UnpackSkipPixels = GL_UNPACK_SKIP_PIXELS,
        UnpackSkipRows = GL_UNPACK_SKIP_ROWS,
        UnpackSwapBytes = GL_UNPACK_SWAP_BYTES,
        //  ZoomX = GL_ZOOM_X,
        //  ZoomY = GL_ZOOM_Y,
    };

    // OpenGL Texture mapping
    enum gltexturemap : glenum {
        //  TextureEnv = GL_TEXTURE_ENV,
        //  TextureEnvMode = GL_TEXTURE_ENV_MODE,
        Texture_1d = GL_TEXTURE_1D,
        Texture_2d = GL_TEXTURE_2D,
        TextureWrapS = GL_TEXTURE_WRAP_S,
        TextureWrapT = GL_TEXTURE_WRAP_T,
        TextureMagFilter = GL_TEXTURE_MAG_FILTER,
        TextureMinFilter = GL_TEXTURE_MIN_FILTER,
        // TextureEnvColor = GL_TEXTURE_ENV_COLOR,
        // TextureGenS = GL_TEXTURE_GEN_S,
        // TextureGenT = GL_TEXTURE_GEN_T,
        // TextureGenR = GL_TEXTURE_GEN_R,
        // TextureGenQ = GL_TEXTURE_GEN_Q,
        // TextureGenMode = GL_TEXTURE_GEN_MODE,
        TextureBorderColor = GL_TEXTURE_BORDER_COLOR,
        TextureWidth = GL_TEXTURE_WIDTH,
        TextureHeight = GL_TEXTURE_HEIGHT,
        // TextureBorder = GL_TEXTURE_BORDER,
        // TextureComponents = GL_TEXTURE_COMPONENTS,
        TextureRedSize = GL_TEXTURE_RED_SIZE,
        TextureGreenSize = GL_TEXTURE_GREEN_SIZE,
        TextureBlueSize = GL_TEXTURE_BLUE_SIZE,
        TextureAlphaSize = GL_TEXTURE_ALPHA_SIZE,
        // TextureLuminanceSize = GL_TEXTURE_LUMINANCE_SIZE,
        // TextureIntensitySize = GL_TEXTURE_INTENSITY_SIZE,
        NearestMipmapNearest = GL_NEAREST_MIPMAP_NEAREST,
        NearestMipmapLinear = GL_NEAREST_MIPMAP_LINEAR,
        LinearMipmapNearest = GL_LINEAR_MIPMAP_NEAREST,
        LinearMipmapLinear = GL_LINEAR_MIPMAP_LINEAR,
        //  ObjectLinear = GL_OBJECT_LINEAR,
        //  ObjectPlane = GL_OBJECT_PLANE,
        //  EyeLinear = GL_EYE_LINEAR,
        EyePlane = GL_EYE_PLANE,
        // SphereMap = GL_SPHERE_MAP,
        // Decal = GL_DECAL,
        // Modulate = GL_MODULATE,
        Nearest = GL_NEAREST,
        Repeat = GL_REPEAT,
        // Clamp = GL_CLAMP,
        // S = GL_S,
        // T = GL_T,
        // R = GL_R,
        // Q = GL_Q,
    };

    // OpenGL Utility
    enum glutils : glenum {
        Vendor = GL_VENDOR,
        Renderer = GL_RENDERER,
        Version = GL_VERSION,
        Extensions = GL_EXTENSIONS,
    };

    // OpenGL Errors
    enum glerror : glenum {
        NoError = GL_NO_ERROR,
        InvalidEnum = GL_INVALID_ENUM,
        InvalidValue = GL_INVALID_VALUE,
        InvalidOperation = GL_INVALID_OPERATION,
        StackOverflow = GL_STACK_OVERFLOW,
        StackUnderflow = GL_STACK_UNDERFLOW,
        OutOfMemory = GL_OUT_OF_MEMORY,
    };

    // OpenGL glPush/PopAttrib bits
    enum glbit : glenum {
        // CurrentBit = GL_CURRENT_BIT,
        // PointBit = GL_POINT_BIT,
        // LineBit = GL_LINE_BIT,
        // PolygonBit = GL_POLYGON_BIT,
        // PolygonStippleBit = GL_POLYGON_STIPPLE_BIT,
        // PixelModeBit = GL_PIXEL_MODE_BIT,
        // LightingBit = GL_LIGHTING_BIT,
        // FogBit = GL_FOG_BIT,
        DepthBufferBit = GL_DEPTH_BUFFER_BIT,
        // AccumBufferBit = GL_ACCUM_BUFFER_BIT,
        StencilBufferBit = GL_STENCIL_BUFFER_BIT,
        // ViewportBit = GL_VIEWPORT_BIT,
        // TransformBit = GLRANSFORM_BIT,
        // EnableBit = GL_ENABLE_BIT,
        ColorBufferBit = GL_COLOR_BUFFER_BIT,
        // HintBit = GL_HINT_BIT,
        // EvalBit = GL_EVAL_BIT,
        // ListBit = GL_LIST_BIT,
        // TextureBit = GL_TEXTURE_BIT,
        // ScissorBit = GL_SCISSOR_BIT,
        // AllAttribBits = GL_ALL_ATTRIB_BITS,
    };

    // OpenGL OpenGL 1.1
    enum gllegacy : glenum {
        // ProxyTexture1D = GL_PROXYEXTURE_1D,
        // ProxyTexture2D = GL_PROXYEXTURE_2D,
        // TexturePriority = GL_TEXTURE_PRIORITY,
        // TextureResident = GL_TEXTURE_RESIDENT,
        TextureBinding1D = GL_TEXTURE_BINDING_1D,
        TextureBinding2D = GL_TEXTURE_BINDING_2D,
        TextureInternalFormat = GL_TEXTURE_INTERNAL_FORMAT,
        // Alpha4 = GL_ALPHA4,
        // Alpha8 = GL_ALPHA8,
        // Alpha12 = GL_ALPHA12,
        // Alpha16 = GL_ALPHA16,
        // Luminance4 = GL_LUMINANCE4,
        // Luminance8 = GL_LUMINANCE8,
        // Luminance12 = GL_LUMINANCE12,
        // Luminance16 = GL_LUMINANCE16,
        // Luminance4Alpha4 = GL_LUMINANCE4_ALPHA4,
        // Luminance6Alpha2 = GL_LUMINANCE6_ALPHA2,
        // Luminance8Alpha8 = GL_LUMINANCE8_ALPHA8,
        // Luminance12Alpha4 = GL_LUMINANCE12_ALPHA4,
        // Luminance12Alpha12 = GL_LUMINANCE12_ALPHA12,
        // Luminance16Alpha16 = GL_LUMINANCE16_ALPHA16,
        // Intensity = GL_INTENSITY,
        // Intensity4 = GL_INTENSITY4,
        // Intensity8 = GL_INTENSITY8,
        // Intensity12 = GL_INTENSITY12,
        // Intensity16 = GL_INTENSITY16,
        R3G3B2 = GL_R3_G3_B2,
        RGB4 = GL_RGB4,
        RGB5 = GL_RGB5,
        RGB8 = GL_RGB8,
        RGB10 = GL_RGB10,
        RGB12 = GL_RGB12,
        RGB16 = GL_RGB16,
        RGBA2 = GL_RGBA2,
        RGBA4 = GL_RGBA4,
        RGB5A1 = GL_RGB5_A1,
        RGBA8 = GL_RGBA8,
        RGB10A2 = GL_RGB10_A2,
        RGBA12 = GL_RGBA12,
        RGBA16 = GL_RGBA16,
        // ClientPixelStoreBit = GL_CLIENT_PIXEL_STORE_BIT,
        // ClientVertexArrayBit = GL_CLIENT_VERTEX_ARRAY_BIT,
        // AllClientAttribBits = GL_ALL_CLIENT_ATTRIB_BITS,
        // ClientAllAttribBits = GL_CLIENT_ALL_ATTRIB_BITS,
    };
}
