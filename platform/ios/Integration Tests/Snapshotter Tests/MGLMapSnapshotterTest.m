#import "MGLMapViewIntegrationTest.h"

@interface MGLMapSnapshotterTest : MGLMapViewIntegrationTest

@end

@implementation MGLMapSnapshotterTest

- (MGLMapSnapshotter*)startSnapshotterWithCoordinates:(CLLocationCoordinate2D)coordinates completion:(MGLMapSnapshotCompletionHandler)completion {
    // Create snapshot options
    MGLMapCamera* mapCamera    = [[MGLMapCamera alloc] init];
    mapCamera.pitch            = 20;
    mapCamera.centerCoordinate = coordinates;
    MGLMapSnapshotOptions* options = [[MGLMapSnapshotOptions alloc] initWithStyleURL:[MGLStyle satelliteStreetsStyleURL]
                                                                              camera:mapCamera
                                                                                size:self.mapView.bounds.size];
    options.zoomLevel = 10;

    // Create and start the snapshotter
    MGLMapSnapshotter* snapshotter = [[MGLMapSnapshotter alloc] initWithOptions:options];
    [snapshotter startWithCompletionHandler:completion];

    return snapshotter;
}

- (void)testMultipleSnapshotters {

    NSString *accessToken = [[NSProcessInfo processInfo] environment][@"MAPBOX_ACCESS_TOKEN"];
    if (!accessToken) {
        printf("warning: MAPBOX_ACCESS_TOKEN env var is required for this test - skipping.\n");
        return;
    }

    [MGLAccountManager setAccessToken:accessToken];

    NSUInteger numSnapshots = 20;
    NSMutableArray *expectations = [NSMutableArray arrayWithCapacity:numSnapshots];
    NSMutableSet *snapshots = [NSMutableSet setWithCapacity:numSnapshots];

    __weak __typeof(self) weakself = self;
    __block CLLocationCoordinate2D coordinates = CLLocationCoordinate2DMake(30.0, 30.0);
    for (NSInteger run = 0; run < numSnapshots; run++) {
        NSString *expectationTitle = [NSString stringWithFormat:@"Snapshot %ld", run];
        XCTestExpectation *expectation = [self expectationWithDescription:expectationTitle];
        [expectations addObject:expectation];

        MGLMapSnapshotter *snapshotter = [self startSnapshotterWithCoordinates:coordinates
                                                                    completion:^(MGLMapSnapshot * _Nullable snapshot, NSError * _Nullable error) {

                                                                        __typeof(self) strongself = weakself;
                                                                        MGLTestAssertNotNil(strongself, strongself);

                                                                        MGLTestAssertNotNil(strongself, snapshot);
                                                                        MGLTestAssertNotNil(strongself, snapshot.image);
                                                                        MGLTestAssertNil(strongself, error, @"Snapshot should not error with: %@", error);

                                                                        // Change this back to XCTAttachmentLifetimeDeleteOnSuccess when we're sure this
                                                                        // test is passing.
                                                                        XCTAttachment *attachment = [XCTAttachment attachmentWithImage:snapshot.image];
                                                                        attachment.lifetime = XCTAttachmentLifetimeKeepAlways;
                                                                        [strongself addAttachment:attachment];

                                                                        [snapshots addObject:snapshot];
                                                                        [expectation fulfill];
                                                                    }];

        XCTAssertNotNil(snapshotter);

        coordinates.latitude += 1.0;
        coordinates.longitude += 1.0;
    }

    [self waitForExpectations:expectations timeout:60.0];

    XCTAssert(snapshots.count == expectations.count);
}

@end
