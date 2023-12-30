//
//  ContentView.swift
//  rmrfprocursus
//
//  Created by Nick Chan on 31/12/2023.
//

import SwiftUI
import UIKit

struct ContentView: View {
    @State private var message = "";
    var body: some View {
        VStack {
            Text(message).padding(8);
            Button(action: {
                let DeleteAction = UIAlertAction(title: "Delete",
                                     style: .default) { (action) in
                    DispatchQueue.main.async {
                        let unsafePointer = doit();
                        message = String(cString: unsafePointer!);
                        free(unsafePointer);
                    }
                }
                DeleteAction.setValue(UIColor.red, forKey: "titleTextColor");
                let CancelAction = UIAlertAction(title: "Cancel", style: .default) { (action) in }
                let alert = UIAlertController(title: "Confirm Deletion",
                                             message: "Do you really want to delete all jailbreak files at /var/jb?",
                                              preferredStyle: .alert);
                alert.addAction(DeleteAction);
                alert.addAction(CancelAction);
                UIApplication.shared.windows.first!.rootViewController!.present(alert, animated: true, completion: nil)
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
