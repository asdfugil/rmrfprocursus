//
//  ContentView.swift
//  rmrfprocursus
//
//  Created by Nick Chan on 31/12/2023.
//

import SwiftUI

struct ContentView: View {
    @State private var message = "";
    var body: some View {
        VStack {
            Text(message).padding(8);
            Button(action: {
                DispatchQueue.main.async {
                    let unsafePointer = doit();
                    message = String(cString: unsafePointer!);
                    free(unsafePointer);
                }
            }) {
                Text("delete")
            }
        }
        .padding()
    }
}

#Preview {
    ContentView()
}
