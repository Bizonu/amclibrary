Project name:
		AMC Library & AMC Tools

Author(s):
		Chiuta Adrian Marius

License:
		Licensed under the Apache License, Version 2.0 (the "License");
		you may not use this file except in compliance with the License.
		You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
		Unless required by applicable law or agreed to in writing,
		software distributed under the License is distributed on an "AS IS" BASIS,
		WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
		See the License for the specific language governing permissions and
		limitations under the License.

About:
		This project contains the files needed to build the AMC Lib and AMC Tools.
		The project needs the following programs/SDKs to be installed:

			- Visual Studio 2010 with C++ support

			- Microsoft DirectX SDK (any realease older than 2008 should be ok - was tested with the August 2009 & June 2010 release).
			  Be sure that the Enviroment Variable DXSDK_DIR is set to the folder in which the SDK is installed.

			- Flash Player ActiveX (tested with version 10 & 11). This is needed by AMC Tool Results Viewer which
			  uses Open Flash Chart to display some graphs.

			- Nullsoft Scriptable Install System - NSIS for setup creation (tested with version 2.46)

		If another DirectX SDK or Flash Player ActiveX is used, then the new files should be placed into the REDIST folder !!!


Used Libraries/Software:
		- Open Flash Chart 			- http://teethgrinder.co.uk/open-flash-chart/
		- Windows Template Library 		- http://sourceforge.net/projects/wtl/
		- Nullsoft Scriptable Install System 	- http://nsis.sourceforge.net/Main_Page
		- Adobe Flash Player 			- http://get.adobe.com/flashplayer/
		- Microsoft DirectX SDK			- http://msdn.microsoft.com/en-us/directx/aa937788
		- Microsoft Visual Studio		- http://msdn.microsoft.com/en-us/vstudio/aa718325
