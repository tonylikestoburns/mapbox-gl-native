#import <Mapbox/Mapbox.h>
#import <XCTest/XCTest.h>

static NSString * const MGLTestAnnotationReuseIdentifer = @"MGLTestAnnotationReuseIdentifer";

@interface MGLCustomAnnotationView : MGLAnnotationView

@end

@implementation MGLCustomAnnotationView

- (instancetype)initWithReuseIdentifier:(NSString *)reuseIdentifier {
    return [super initWithReuseIdentifier:@"reuse-id"];
}

@end

@interface MGLAnnotationView (Test)

@property (nonatomic) MGLMapView *mapView;
@property (nonatomic, readwrite) MGLAnnotationViewDragState dragState;
- (void)setDragState:(MGLAnnotationViewDragState)dragState;

@end

@interface MGLMapView (Test)
@property (nonatomic) UIView<MGLCalloutView> *calloutViewForSelectedAnnotation;
@end

@interface MGLTestAnnotation : NSObject <MGLAnnotation>
@property (nonatomic, assign) CLLocationCoordinate2D coordinate;
@end

@implementation MGLTestAnnotation
@end

@interface MGLTestCalloutView: UIView<MGLCalloutView>
@property (nonatomic) BOOL didCallDismissCalloutAnimated;
@property (nonatomic, strong) id <MGLAnnotation> representedObject;
@property (nonatomic, strong) UIView *leftAccessoryView;
@property (nonatomic, strong) UIView *rightAccessoryView;
@property (nonatomic, weak) id<MGLCalloutViewDelegate> delegate;
@end

@implementation MGLTestCalloutView

- (void)dismissCalloutAnimated:(BOOL)animated
{
    _didCallDismissCalloutAnimated = YES;
}

- (void)presentCalloutFromRect:(CGRect)rect inView:(UIView *)view constrainedToView:(UIView *)constrainedView animated:(BOOL)animated { }

@end

@interface MGLAnnotationViewTests : XCTestCase <MGLMapViewDelegate>
@property (nonatomic) XCTestExpectation *expectation;
@property (nonatomic) MGLMapView *mapView;
@property (nonatomic, weak) MGLAnnotationView *annotationView;
@end

@implementation MGLAnnotationViewTests

- (void)setUp
{
    [super setUp];
    _mapView = [[MGLMapView alloc] initWithFrame:CGRectMake(0, 0, 64, 64)];
    _mapView.delegate = self;
}

- (void)testAnnotationView
{
    _expectation = [self expectationWithDescription:@"annotation property"];

    MGLTestAnnotation *annotation = [[MGLTestAnnotation alloc] init];
    [_mapView addAnnotation:annotation];

    [self waitForExpectationsWithTimeout:1 handler:nil];

    XCTAssert(_mapView.annotations.count == 1, @"number of annotations should be 1");
    XCTAssertNotNil(_annotationView.annotation, @"annotation property should not be nil");
    XCTAssertNotNil(_annotationView.mapView, @"mapView property should not be nil");

    MGLTestCalloutView *testCalloutView = [[MGLTestCalloutView  alloc] init];
    _mapView.calloutViewForSelectedAnnotation = testCalloutView;
    _annotationView.dragState = MGLAnnotationViewDragStateStarting;
    XCTAssertTrue(testCalloutView.didCallDismissCalloutAnimated, @"callout view was not dismissed");

    [_mapView removeAnnotation:_annotationView.annotation];

    XCTAssert(_mapView.annotations.count == 0, @"number of annotations should be 0");
    XCTAssertNil(_annotationView.annotation, @"annotation property should be nil");
}

- (void)testCustomAnnotationView
{
    MGLCustomAnnotationView *customAnnotationView = [[MGLCustomAnnotationView alloc] initWithReuseIdentifier:@"reuse-id"];
    XCTAssertNotNil(customAnnotationView);
}

- (void)testAddAnnotationWithBoundaryCoordinates
{
    typedef struct {
        CLLocationDegrees lat;
        CLLocationDegrees lon;
        BOOL expectation;
    } TestParam;

    TestParam params[] = {
//          Lat     Lon     Valid
        {   -91.0,  0.0,    NO},

        // The follow coordinate fails, essentially because the following in projection.hpp
        //
        //      util::LONGITUDE_MAX - util::RAD2DEG * std::log(std::tan(M_PI / 4 + latLng.latitude() * M_PI / util::DEGREES_MAX))
        //
        // boils down to ln(0)
        //
        // It makes sense that -90° latitude (south pole) should be invalid from a projection point
        // of view, but in that case shouldn't +90° (north pole) also be invalid?
        //
        // In Obj-C code, perhaps we need to replace usage of CLLocationCoordinate2DIsValid for an
        // MGL one...

        {   -90.0,  0.0,    YES}, // South pole. Should this really be considered an invalid coordinate?

        // The rest for completeness
        {   -89.0,  0.0,    YES},
        {   90.0,   0.0,    YES}, // North pole. Similarly, should this one be considered invalid?
        {   91.0,   0.0,    NO},

        {   0.0,    -181.0, NO},
        {   0.0,    -180.0, YES},
        {   0.0,    180.0,  YES},
        {   0.0,    181.0,  NO},
    };

    for (int i = 0; i < sizeof(params)/sizeof(params[0]); i++) {
        TestParam param = params[i];

        // Essentially a deconstructed -[MGLMapView convertCoordinate:toPointToView]
        CLLocationCoordinate2D coordinate = CLLocationCoordinate2DMake(param.lat, param.lon);
        NSString *coordDesc = [NSString stringWithFormat:@"(%0.1f,%0.1f)", param.lat, param.lon];

        XCTAssert(CLLocationCoordinate2DIsValid(coordinate) == param.expectation, @"Unexpected valid result for coordinate %@", coordDesc);

        CGPoint point = [_mapView convertCoordinate:coordinate toPointToView:_mapView];
        (void)point;
        XCTAssert(isnan(point.x) != param.expectation, @"Unexpected point.x for coordinate %@", coordDesc);
        XCTAssert(isnan(point.y) != param.expectation, @"Unexpected point.y for coordinate %@", coordDesc);

        if (param.expectation) {
            // If we expect a valid coordinate, let's finally try to add an annotation

            // The above method is called by the following, which will trigger CALayer to raise an
            // exception
            MGLTestAnnotation *annotation = [[MGLTestAnnotation alloc] init];
            annotation.coordinate = coordinate;

            @try {
                [_mapView addAnnotation:annotation];
            }
            @catch (NSException *e) {
                XCTFail("addAnnotation triggered exception: %@ for coordinate %@", e, coordDesc);
            }
        }
    }
}



- (MGLAnnotationView *)mapView:(MGLMapView *)mapView viewForAnnotation:(id<MGLAnnotation>)annotation
{
    MGLAnnotationView *annotationView = [mapView dequeueReusableAnnotationViewWithIdentifier:MGLTestAnnotationReuseIdentifer];

    if (!annotationView)
    {
        annotationView = [[MGLAnnotationView alloc] initWithAnnotation:annotation reuseIdentifier:MGLTestAnnotationReuseIdentifer];
    }

    _annotationView = annotationView;

    return annotationView;
}

- (void)mapView:(MGLMapView *)mapView didAddAnnotationViews:(NSArray<MGLAnnotationView *> *)annotationViews
{
    [_expectation fulfill];
}

@end
