#import <Cocoa/Cocoa.h>
#import <ApplicationServices/ApplicationServices.h>

@interface XenonAppDelegate : NSObject <NSApplicationDelegate>
@property (nonatomic, strong) NSStatusItem *statusItem;
@property (nonatomic, strong) NSMenuItem   *statusLabel;
@property (nonatomic, strong) NSMenuItem   *warningLabel;
@property (nonatomic, strong) NSMenuItem   *modeLabel;
@property (nonatomic, strong) NSTask       *cliTask;
@property (nonatomic, strong) NSMutableString *lineBuffer;
@property (nonatomic, copy)   NSString    *deviceName;
@property (nonatomic) BOOL gamepadMode;
@property (nonatomic) BOOL accessibilityWarning;
@end

@implementation XenonAppDelegate

- (NSImage *)guitarIcon {
    NSImage *icon = nil;
    if (@available(macOS 11.0, *)) {
        icon = [NSImage imageWithSystemSymbolName:@"guitars.fill"
                         accessibilityDescription:@"Xenon360"];
        if (!icon) {
            icon = [NSImage imageWithSystemSymbolName:@"gamecontroller.fill"
                             accessibilityDescription:@"Xenon360"];
        }
    }
    if (icon) icon.template = YES;
    return icon;
}

- (BOOL)requestAccessibilityIfNeeded {
    // AXIsProcessTrustedWithOptions(prompt=true) is what makes macOS show
    // the system "<App> wants to control your computer" popup. If the app's
    // bundle ID is already in the TCC database (granted or denied), it just
    // returns the cached result without re-prompting. So calling this on
    // every launch is safe: it only annoys the user the very first time.
    //
    // We call it from the .app's main process (not the CLI subprocess) so
    // TCC attributes the request to dev.vesanerie.xenon360, the bundle ID
    // the user actually sees in Settings.
    NSDictionary *options = @{(__bridge id)kAXTrustedCheckOptionPrompt: @YES};
    return AXIsProcessTrustedWithOptions((__bridge CFDictionaryRef)options);
}

- (void)applicationDidFinishLaunching:(NSNotification *)note {
    self.lineBuffer = [NSMutableString string];
    self.gamepadMode = NO;

    // Prompt for Accessibility before spawning the CLI. The system popup
    // will appear if this bundle ID has never been granted permission.
    [self requestAccessibilityIfNeeded];

    self.statusItem = [[NSStatusBar systemStatusBar]
                       statusItemWithLength:NSVariableStatusItemLength];

    NSImage *icon = [self guitarIcon];
    if (icon) {
        self.statusItem.button.image = icon;
    } else {
        self.statusItem.button.title = @"X360";
    }

    NSMenu *menu = [[NSMenu alloc] init];

    self.statusLabel = [[NSMenuItem alloc] initWithTitle:@"Demarrage..."
                                                  action:nil keyEquivalent:@""];
    [self.statusLabel setEnabled:NO];
    [menu addItem:self.statusLabel];

    self.warningLabel = [[NSMenuItem alloc] initWithTitle:@""
                                                   action:nil keyEquivalent:@""];
    [self.warningLabel setEnabled:NO];
    [self.warningLabel setHidden:YES];
    [menu addItem:self.warningLabel];

    self.modeLabel = [[NSMenuItem alloc] initWithTitle:@"Mode : Clavier"
                                                action:nil keyEquivalent:@""];
    [self.modeLabel setEnabled:NO];
    [menu addItem:self.modeLabel];

    [menu addItem:[NSMenuItem separatorItem]];

    NSMenuItem *openSettings = [[NSMenuItem alloc]
        initWithTitle:@"Ouvrir Reglages Accessibilite..."
               action:@selector(openAccessibilitySettings:)
        keyEquivalent:@""];
    openSettings.target = self;
    [menu addItem:openSettings];

    NSMenuItem *aboutItem = [[NSMenuItem alloc] initWithTitle:@"A propos de Xenon360"
                                                       action:@selector(openAbout:)
                                                keyEquivalent:@""];
    aboutItem.target = self;
    [menu addItem:aboutItem];

    [menu addItem:[NSMenuItem separatorItem]];

    NSMenuItem *quitItem = [[NSMenuItem alloc] initWithTitle:@"Quitter Xenon360"
                                                      action:@selector(quit:)
                                               keyEquivalent:@"q"];
    quitItem.target = self;
    [menu addItem:quitItem];

    self.statusItem.menu = menu;

    [self launchCLI];
}

- (void)launchCLI {
    NSString *cliPath = [[NSBundle mainBundle] pathForResource:@"xenon360" ofType:nil];
    if (!cliPath) {
        self.statusLabel.title = @"Erreur : binaire CLI introuvable";
        return;
    }

    self.cliTask = [[NSTask alloc] init];
    self.cliTask.launchPath = cliPath;
    self.cliTask.arguments = self.gamepadMode ? @[@"-g"] : @[];

    NSPipe *outPipe = [NSPipe pipe];
    self.cliTask.standardOutput = outPipe;
    self.cliTask.standardError = outPipe;

    __weak typeof(self) weakSelf = self;
    [outPipe.fileHandleForReading setReadabilityHandler:^(NSFileHandle *fh) {
        NSData *data = [fh availableData];
        if (data.length == 0) return;
        NSString *text = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
        if (!text) return;
        dispatch_async(dispatch_get_main_queue(), ^{
            [weakSelf ingestOutput:text];
        });
    }];

    self.cliTask.terminationHandler = ^(NSTask *t) {
        dispatch_async(dispatch_get_main_queue(), ^{
            __strong typeof(weakSelf) strongSelf = weakSelf;
            // If the user is quitting the app, quit:: nils out terminationHandler
            // and terminates the task, but the handler may already be in flight.
            // Guard against re-launching during shutdown.
            if (!strongSelf || !strongSelf.cliTask) return;
            strongSelf.statusLabel.title = @"Cherche guitare...";
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC),
                           dispatch_get_main_queue(), ^{
                __strong typeof(weakSelf) s = weakSelf;
                if (s && s.cliTask) [s launchCLI];
            });
        });
    };

    NSError *err = nil;
    if (![self.cliTask launchAndReturnError:&err]) {
        self.statusLabel.title = [NSString stringWithFormat:@"Erreur : %@",
                                  err.localizedDescription];
    } else {
        self.statusLabel.title = @"Cherche guitare...";
    }
}

- (void)ingestOutput:(NSString *)chunk {
    [self.lineBuffer appendString:chunk];
    NSArray *lines = [self.lineBuffer componentsSeparatedByString:@"\n"];
    for (NSUInteger i = 0; i + 1 < lines.count; i++) {
        [self parseLine:lines[i]];
    }
    self.lineBuffer = [[lines lastObject] mutableCopy];
}

- (void)parseLine:(NSString *)line {
    if ([line hasPrefix:@"Detecte : "]) {
        self.deviceName = [line substringFromIndex:[@"Detecte : " length]];
    } else if ([line containsString:@"No known device found"] ||
               [line containsString:@"Branche ta guitare"]) {
        self.deviceName = nil;
    } else if ([line containsString:@"Device debranche"]) {
        self.deviceName = nil;
    } else if ([line containsString:@"ATTENTION : permission Accessibilite NON accordee"]) {
        self.accessibilityWarning = YES;
    } else if ([line containsString:@"OK : permission Accessibilite accordee"]) {
        self.accessibilityWarning = NO;
    } else {
        return;
    }
    [self refreshUI];
}

- (void)refreshUI {
    if (self.deviceName) {
        self.statusLabel.title = [NSString stringWithFormat:@"Connectee : %@", self.deviceName];
    } else {
        self.statusLabel.title = @"Aucune guitare detectee";
    }
    if (self.accessibilityWarning) {
        self.warningLabel.title = @"⚠ Autorise Xenon360 dans Accessibilite";
        [self.warningLabel setHidden:NO];
    } else {
        [self.warningLabel setHidden:YES];
    }

    NSImage *icon = [self guitarIcon];
    if (icon && self.statusItem.button.image) {
        self.statusItem.button.image = icon;
    }
}

- (void)openAccessibilitySettings:(id)sender {
    NSURL *url = [NSURL URLWithString:
        @"x-apple.systempreferences:com.apple.preference.security?Privacy_Accessibility"];
    [[NSWorkspace sharedWorkspace] openURL:url];
}

- (void)openAbout:(id)sender {
    NSURL *url = [NSURL URLWithString:@"https://github.com/Vesanerie/Xenon360"];
    [[NSWorkspace sharedWorkspace] openURL:url];
}

- (void)quit:(id)sender {
    NSTask *t = self.cliTask;
    self.cliTask = nil;
    if (t) {
        t.terminationHandler = nil;
        if (t.isRunning) [t terminate];
    }
    [NSApp terminate:nil];
}

@end

int main(int argc, const char *argv[]) {
    (void)argc; (void)argv;
    @autoreleasepool {
        NSApplication *app = [NSApplication sharedApplication];
        XenonAppDelegate *delegate = [[XenonAppDelegate alloc] init];
        app.delegate = delegate;
        [app setActivationPolicy:NSApplicationActivationPolicyAccessory];
        [app run];
    }
    return 0;
}
