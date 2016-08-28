#include <Squirrel/Squirrel.h>
#define QT_NO_SIGNALS_SLOTS_KEYWORDS
#include <QDebug>
#include <QDateTime>
#undef QT_NO_SIGNALS_SLOTS_KEYWORDS
#include "version.h"

#include "SquirrelAutoUpdater.h"

namespace core {

class SquirrelAutoUpdater::Private {
 public:
  SQRLUpdater* updater;
};

SquirrelAutoUpdater::SquirrelAutoUpdater() {
  d = std::make_unique<Private>();
}

void SquirrelAutoUpdater::initialize() {
  @autoreleasepool {
    NSURLComponents* components = [[NSURLComponents alloc] init];

    components.scheme = @"https";
    components.host = @"silkedit-release-server.herokuapp.com";
    components.path = @"/update";

#ifdef BUILD_EDGE
    NSString* channel = @"edge";
#else
    NSString* channel = @"stable";
#endif

#if defined Q_OS_MAC
    NSString* platform = @"mac";
#elif defined Q_OS_WIN64
    NSString* platform = @"windows_x64";
#else
    NSString* platform = @"windows_x86";
#endif

    components.query =
        [[NSString stringWithFormat:@"channel=%1$@&version=%2$@&build=%3$@&platform=%4$@", channel,
                                    @VERSION, @BUILD, platform]
            stringByAddingPercentEncodingWithAllowedCharacters:NSCharacterSet
                                                                   .URLQueryAllowedCharacterSet];

    @try {
      NSMutableURLRequest* request = [[NSMutableURLRequest alloc] initWithURL:components.URL];
      [request addValue:@"Bearer BymXwSHDJa" forHTTPHeaderField:@"Authorization"];
      d->updater = [[SQRLUpdater alloc] initWithUpdateRequest:request];
    } @catch (NSException* exception) {
      emit updateError(exception.reason.UTF8String);
      return;
    }

    Q_ASSERT(d->updater);
    [[d->updater rac_valuesForKeyPath:@"state" observer:d->updater]
        subscribeNext:^(NSNumber* stateNumber) {
          int state = [stateNumber integerValue];
          if (state == SQRLUpdaterStateCheckingForUpdate) {
            emit checkingForUpdate();
          } else if (state == SQRLUpdaterStateDownloadingUpdate) {
            emit updateAvailable();
          }
        }];
  }
}

void SquirrelAutoUpdater::checkForUpdates() {
  if (!d->updater) {
    emit updateError("updater is null");
    return;
  }

  [[[[d->updater.checkForUpdatesCommand execute:nil]
      // Send a `nil` after everything...
      concat:[RACSignal return:nil]]
      // But only take the first value. If an update is sent, we'll get that.
      // Otherwise, we'll get our inserted `nil` value.
      take:1] subscribeNext:^(SQRLDownloadedUpdate* downloadedUpdate) {
    if (downloadedUpdate) {
      SQRLUpdate* update = downloadedUpdate.update;
      // There is a new update that has been downloaded.
      emit updateDownloaded(
          QString::fromNSString(update.releaseNotes), QString::fromNSString(update.releaseName),
          QDateTime::fromMSecsSinceEpoch(update.releaseDate.timeIntervalSince1970 * 1000),
          QString::fromNSString(update.updateURL.absoluteString));
    } else {
      // When the completed event is sent with no update, then we know there
      // is no update available.
      emit updateNotAvailable();
    }
  }
      error:^(NSError* error) {
        NSMutableString* failureString =
            [NSMutableString stringWithString:error.localizedDescription];
        if (error.localizedFailureReason) {
          [failureString appendString:@": "];
          [failureString appendString:error.localizedFailureReason];
        }
        if (error.localizedRecoverySuggestion) {
          if (![failureString hasSuffix:@"."])
            [failureString appendString:@"."];
          [failureString appendString:@" "];
          [failureString appendString:error.localizedRecoverySuggestion];
        }
        emit updateError(failureString.UTF8String);
      }];
}

}  // namespace core
