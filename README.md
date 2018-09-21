# apama-base64-plugin
Apama EPL plugin and connectivity codec for en/decoding data in base64.

## Supported Apama version

This works with Apama 10.3.0.1 or later

## Building the plugin

In an Apama command prompt on Linux run:

    mkdir -p $APAMA_WORK/lib $APAMA_WORK/monitors
    g++ -std=c++11 -o $APAMA_WORK/lib/libBase64Plugin.so -I$APAMA_HOME/include -L$APAMA_HOME/lib -lapclient -I. -shared -fPIC Base64Plugin.cpp
	 cp eventdefinitions/Base64Plugin.mon $APAMA_WORK/monitors/Base64Plugin.mon

On Windows run:

    g++ -std=c++11 -o %APAMA_WORK%\lib\Base64Plugin.dll -I%APAMA_HOME%\include -L%APAMA_HOME%\lib -lapclient -I. -shared Base64Plugin.cpp
	 copy eventdefinitions\Base64Plugin.mon %APAMA_WORK%\monitors\Base64Plugin.mon

To generate the Apama documentation for the Base64Plugin module run this command on Linux:

    java -jar $APAMA_HOME/lib/ap-generate-apamadoc.jar doc eventdefinitions

Or on Windows:

    java -jar %APAMA_HOME%\lib\ap-generate-apamadoc.jar doc eventdefinitions

## Building using Docker

There is a provided Dockerfile which will build the plugin, run tests and produce an image which is your base image, plus the Base64 plugin. Application images can then be built from this image. To build the image run:

    docker build -t apama_with_csv_plugin .

By default the public docker images from Docker Store for 10.3 will be used (once 10.3 has been released). To use another version run:

    docker build -t apama_with_csv_plugin --build-arg APAMA_VERSION=10.1 .

To use custom images from your own repository then use:

    docker build -t apama_with_csv_plugin --build-arg APAMA_BUILDER=builderimage --build-arg APAMA_IMAGE=runtimeimage .

## Running tests

To run the tests for the plugin you will need to use an Apama command prompt to run the tests from within the tests directory:

    pysys run

## Use from EPL

From EPL you should inject the Base64Plugin.mon file, then used the static actions on Base64Plugin to encode and decode strings as base64:

    string data := "Hello World";
	 string encodedData := com.apamax.Base64Plugin.encode(data);
    data := com.apamax.Base64Plugin.decode(encodedData);

If you try to decode a string which is not base-64 encoded then decode will throw an exception. Strings bust be in UTF-8 format when calling from EPL.

## Use from Connectivity plugins

As a codec in a connectivity chain you will need to first import the plugin into your configuration:

    connectivityPlugins:
	   base64Codec:
        libraryName: Base64Plugin
		  class: Base64Codec

You can now use the Base64 codec in a chain definition:

    base64Chain:
	    - apama.eventMap
       - mapperCodec:
		    # ... mapping configuration
		 - stringCodec
		 - base64Codec
		 - someTransport

The Base64 codec converts an arbitrary byte buffer on the host side to a base64-encoded byte buffer on the transport side. Thus we use it after the string codec. By default the Base64 codec will apply to all messages.

The CSV codec takes the following options:

    base64Chain
	   # ...
	   - base64Codec:
          filterOnTransferEncoding: true # if true, only apply to messages with the specified transfere encoding set. Default is false
	   # ...   

If this is set to true, then the HTTP header content-transfer-encoding is inspected and the transform is only applied if it is base64.

To use this feature you will need to ensure that the header is set appropriately in both directions. If you need to encode messages to the transport in base64 then you can use the mapper codec:

    base64Chain
	   # ...
	   - mapperCodec:
		    "*":
			    towardsTransport:
				    defaultValue:
					    - metadata.http.headers.content-transfer-encoding: base64
		# ...
      - base64Codec:
		    filterOnTransferEncoding: true
	   # ...


