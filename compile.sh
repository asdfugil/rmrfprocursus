#!/bin/sh
	rm -rf build
	xcodebuild clean build -sdk iphoneos -configuration Release CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO CODE_SIGNING_ALLOWED="NO"
	rm -rf Payload FUCK.tipa
	mkdir Payload
	cp -r build/Release-iphoneos/rmrfprocursus.app Payload
	ldid -Sent.xml -Cadhoc Payload/rmrfprocursus.app
	zip -r9 FUCK.tipa Payload/ -x "*.DS_Store"
	open .
