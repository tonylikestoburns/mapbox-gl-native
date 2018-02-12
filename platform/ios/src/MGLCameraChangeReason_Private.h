#import "MGLFoundation.h"

/**
 Internal bitmask values used to represent ongoing gestures and API calls that triggers a camera
 change.

 At the end of a camera movement, the resulting bitmask is mapped to one of the public
 MGLCameraChangeReason enum values and passed to the delegate camera change methods.
 */
typedef NS_OPTIONS(NSUInteger, MGLCameraChangeReasonBitmask)
{
    /// The default value. Set at the start, and reset when a camera move has completed.
    MGLCameraChangeReasonBitmaskNone = 0,

    /// Set when a public API that moves the camera is called. This may be set for some gestures,
    /// for example, when the user taps the compass to reset north.
    MGLCameraChangeReasonBitmaskProgrammatic = 1 << 0,

    /// The user tapped the compass to reset the map orientation so North is up.
    MGLCameraChangeReasonBitmaskResetNorth = 1 << 1,

    /// The user panned the map.
    MGLCameraChangeReasonBitmaskGesturePan = 1 << 2,

    /// The user pinched to zoom in/out.
    MGLCameraChangeReasonBitmaskGesturePinch = 1 << 3,

    /// The user rotated the map.
    MGLCameraChangeReasonBitmaskGestureRotate = 1 << 4,

    /// The user zoomed the map in. (One finger double tap.)
    MGLCameraChangeReasonBitmaskGestureZoomIn = 1 << 5,

    /// The user zoomed the map out. (Two finger single tap.)
    MGLCameraChangeReasonBitmaskGestureZoomOut = 1 << 6,

    /// The user long pressed on the map for a quick zoom. (Single tap, then long press and drag up/down.)
    MGLCameraChangeReasonBitmaskGestureOneFingerZoom = 1 << 7,

    // The user panned with two fingers to tilt the map. (Two finger drag.)
    MGLCameraChangeReasonBitmaskGestureTwoFingerDrag = 1 << 8
};
