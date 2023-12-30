//
//  rmrfprocursusApp.swift
//  rmrfprocursus
//
//  Created by Nick Chan on 31/12/2023.
//

import SwiftUI

struct rmrfprocursusApp: App {
    var body: some Scene {
        WindowGroup {
            ContentView()
        }
    }
}

@main
struct EntryPoint {
    static func main() -> Void {
        if (getuid() == 0) {
            exit(doit_forreal());
        } else {
            rmrfprocursusApp.main();
        }
    }
}
