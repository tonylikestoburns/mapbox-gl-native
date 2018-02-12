#import "MGLFoundation.h"

/**
 Reasons describing why a camera move occurred.

 Values of this type are passed to the `MGLMapView`'s delegate in the following methods:

 - `-mapView:shouldChangeFromCamera:toCamera:reason:`
 - `-mapView:regionWillChangeWithReason:animated:`
 - `-mapView:regionIsChangingWithReason:`
 - `-mapView:regionDidChangeWithReason:animated:`

 It's important to note that it's almost impossible to perform a rotate without zooming (in or out),
 so when you want to check for a pan vs a rotation, it's important to ensure you check
 MGLCameraChangeReasonGesturePan against MGLCameraChangeReasonGestureRotateAndZoom.
*/
typedef NS_ENUM(NSUInteger, MGLCameraChangeReason)
{
    /// The reason for the camera change is invalid. This is considered an error.
    MGLCameraChangeReasonInvalid,

    /// Set when a public API that moves the camera is called.
    MGLCameraChangeReasonProgrammatic,

    /// The user panned the map.
    MGLCameraChangeReasonGesturePan,

    /// The user rotated the map. This is also used when the user resets the map orientation by tapping
    /// on the compass. Note that since it's almost impossible to rotate without zooming, you should
    /// also check against MGLCameraChangeReasonGestureRotateAndZoom
    MGLCameraChangeReasonGestureRotate,

    /// The user zoomed the map. This is used when pinching the map, double tapping and for the one
    /// finger drag zoom gesture.
    MGLCameraChangeReasonGestureZoom,

    /// The user panned and rotated the map, without lifting their fingers.
    MGLCameraChangeReasonGesturePanAndRotate,

    /// The user rotated and zoomed the map. You will see this value when rotating the map without a
    /// deliberate zoom. See also MGLCameraChangeReasonGestureRotate.
    MGLCameraChangeReasonGestureRotateAndZoom,

    /// The user panned and zoomed the map, without lifting their fingers.
    MGLCameraChangeReasonGesturePanAndZoom,

    /// The user panned, rotated and zoomed the map, without lifting their fingers.
    MGLCameraChangeReasonGesturePanRotateAndZoom,

    /// The user dragged the map with two fingers to tilt the map.
    MGLCameraChangeReasonGestureTilt
};


