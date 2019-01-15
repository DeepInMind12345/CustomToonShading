// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Engine/EngineTypes.h"
#include "AudioCompressionSettings.h"

#include "AndroidRuntimeSettings.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAndroidRuntimeSettings, Log, All);

UENUM()
namespace EAndroidAntVerbosity
{
	enum Type
	{
		/** Extra quiet logging (-quiet), errors will be logged by second run at normal verbosity. */
		Quiet,

		/** Normal logging (no options) */
		Normal,

		/** Extra verbose logging (-verbose) */
		Verbose,
	};
}

UENUM()
namespace EAndroidScreenOrientation
{
	// IF THIS CHANGES, MAKE SURE TO UPDATE UEDeployAndroid.cs, ConvertOrientationIniValue()!

	enum Type
	{
		/** Portrait orientation (the display is taller than it is wide). */
		Portrait,

		/** Portrait orientation rotated 180 degrees. */
		ReversePortrait,

		/** Use either portrait or reverse portrait orientation, where supported by the device, based on the device orientation sensor. */
		SensorPortrait,

		/** Landscape orientation (the display is wider than it is tall). */
		Landscape,

		/** Landscape orientation rotated 180 degrees. */
		ReverseLandscape,

		/** Use either landscape or reverse landscape orientation, based on the device orientation sensor. */
		SensorLandscape,

		/** Use any orientation the device normally supports, based on the device orientation sensor. */
		Sensor,

		/** Use any orientation (including ones the device wouldn't choose in Sensor mode), based on the device orientation sensor. */
		FullSensor,
	};
}

/** Depth buffer precision preferences */
UENUM()
namespace EAndroidDepthBufferPreference
{
	// IF THIS CHANGES, MAKE SURE TO UPDATE UEDeployAndroid.cs, ConvertDepthBufferIniValue()!

	enum Type
	{
		Default = 0 UMETA(DisplayName = "Default"),
		Bits16 = 16 UMETA(DisplayName = "16-bit"),
		Bits24 = 24 UMETA(DisplayName = "24-bit"),
		Bits32 = 32 UMETA(DisplayName = "32-bit"),
	};
}

/** The default install location for the application */
UENUM()
namespace EAndroidInstallLocation
{
	enum Type
	{
		/** Install your app only on internal device storage */
		InternalOnly,
		/** Install your app on external storage when available */
		PreferExternal,
		/** Internal storage is preferred over external, unless the interal storage is low on space */
		Auto
	};
}

/**
 * Holds the game-specific achievement name and corresponding ID from Google Play services.
 */
USTRUCT()
struct FGooglePlayAchievementMapping
{
	GENERATED_USTRUCT_BODY()

	/** The game-specific achievement name (the one passed in to WriteAchievement calls). */
	UPROPERTY(EditAnywhere, Category = GooglePlayServices)
	FString Name;

	/** The ID of the corresponding achievement, generated by the Google Play developer console. */
	UPROPERTY(EditAnywhere, Category = GooglePlayServices)
	FString AchievementID;
};

/**
 * Holds the game-specific leaderboard name and corresponding ID from Google Play services.
 */
USTRUCT()
struct FGooglePlayLeaderboardMapping
{
	GENERATED_USTRUCT_BODY()

	/** The game-specific leaderboard name (the one passed in to WriteLeaderboards calls). */
	UPROPERTY(EditAnywhere, Category = GooglePlayServices)
	FString Name;

	/** The ID of the corresponding leaderboard, generated by the Google Play developer console. */
	UPROPERTY(EditAnywhere, Category = GooglePlayServices)
	FString LeaderboardID;
};

UENUM()
namespace EAndroidAudio
{
	enum Type
	{
		Default = 0 UMETA(DisplayName = "Default", ToolTip = "This option selects the default encoder."),
		OGG = 1 UMETA(DisplayName = "Ogg Vorbis", ToolTip = "Selects Ogg Vorbis encoding."),
		ADPCM = 2 UMETA(DisplayName = "ADPCM", ToolTip = "This option selects ADPCM lossless encoding.")
	};
}

UENUM()
namespace EGoogleVRMode
{
	enum Type
	{
		Cardboard = 0 UMETA(DisplayName = "Cardboard", ToolTip = "Configure GoogleVR to run in Cardboard-only mode."),
		Daydream = 1 UMETA(DisplayName = "Daydream", ToolTip = "Configure GoogleVR to run in Daydream-only mode. In this mode, app won't be able to run on Non Daydream-ready phone."),
		DaydreamAndCardboard = 2 UMETA(DisplayName = "Daydream & Cardboard", ToolTip = "Configure GoogleVR to run in Daydream mode on Daydream-ready phone and fallback to Cardboard mode on Non Daydream-ready phone.")
	};
}

UENUM()
namespace EGoogleVRCaps
{
	enum Type
	{
		Cardboard = 0 UMETA(DisplayName = "Cardboard", ToolTip = "Head orientation, no controller."),
		Daydream33 = 1 UMETA(DisplayName = "Daydream (3.3 DoF)", ToolTip = "Head orientation, controller orientation. Daydream without positional tracking."),
		Daydream63 = 2 UMETA(DisplayName = "Daydream (6.3 DoF)", ToolTip = "Head position and orientation, controller orientation. Daydream with positional tracking."),
		Daydream66 = 3 UMETA(DisplayName = "Daydream (6.6 DoF)", ToolTip = "Head position and orientation, 2 controllers with position and orientation. Daydream with positional tracking.")
	};
}

UENUM()
namespace EAndroidGraphicsDebugger
{
	enum Type
	{
		None = 0 UMETA(DisplayName = "None"),
		Mali = 1 UMETA(DisplayName = "Mali Graphics Debugger", ToolTip = "Configure for Mali Graphics Debugger."),
		Adreno = 2 UMETA(DisplayName = "Adreno Profiler", ToolTip = "Configure for Adreno Profiler."),
	};
}


/**
 * Implements the settings for the Android runtime platform.
 */
UCLASS(config=Engine, defaultconfig)
class ANDROIDRUNTIMESETTINGS_API UAndroidRuntimeSettings : public UObject
{
public:
	GENERATED_UCLASS_BODY()

	// The official name of the product (same as the name you use on the Play Store web site). Note: Must have at least 2 sections separated by a period and be unique!
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "APK Packaging", Meta = (DisplayName = "Android Package Name ('com.Company.Project', [PROJECT] is replaced with project name)"))
	FString PackageName;

	// The version number used to indicate newer versions in the Store
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "APK Packaging", Meta = (DisplayName = "Store Version (1-2147483647)", ClampMin="1", ClampMax="2147483647"))
	int32 StoreVersion;

	// The visual application name displayed for end users
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "APK Packaging", Meta = (DisplayName = "Application Display Name (app_name), project name if blank"))
	FString ApplicationDisplayName;

	// The visual version displayed for end users
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "APK Packaging", Meta = (DisplayName = "Version Display Name (usually x.y)"))
	FString VersionDisplayName;

	// What OS version the app is allowed to be installed on (do not set this lower than 9)
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "APK Packaging", Meta = (DisplayName = "Minimum SDK Version (9=Gingerbread, 14=Ice Cream Sandwich, 21=Lollipop)"))
	int32 MinSDKVersion;
	
	// What OS version the app is expected to run on (do not set this lower than 9, set to 19 for Gear VR)
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "APK Packaging", Meta = (DisplayName = "Target SDK Version (9=Gingerbread, 14=Ice Cream Sandwich, 21=Lollipop)"))
	int32 TargetSDKVersion;

	// Preferred install location for the application
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "APK Packaging")
	TEnumAsByte<EAndroidInstallLocation::Type> InstallLocation;

	// Use Gradle instead of Ant for Java compiling and APK generation
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "APK Packaging", Meta = (DisplayName = "Enable Gradle instead of Ant"))
	bool bEnableGradle;

	// Enable -Xlint:unchecked and -Xlint:depreciation for Java compiling (Gradle only)
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "APK Packaging", Meta = (DisplayName = "Enable Lint depreciation checks"))
	bool bEnableLint;

	// Should the data be placed into the .apk file instead of a separate .obb file. Amazon requires this to be enabled, but Google Play Store will not allow .apk files larger than 100MB, so only small games will work with this enabled.
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "APK Packaging", Meta = (DisplayName = "Package game data inside .apk?"))
	bool bPackageDataInsideApk;

	// If checked, both batch (.bat) files and shell script (.command) files will be generated, otherwise only done for the current system (default)
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "APK Packaging", Meta = (DisplayName = "Generate install files for all platforms"))
	bool bCreateAllPlatformsInstall;

	// Disable the verification of an OBB file when it is downloaded or on first start when in a distribution build. 
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "APK Packaging", Meta = (DisplayName = "Disable verify OBB on first start/update."))
	bool bDisableVerifyOBBOnStartUp;

	// If checked, OBB is not limited to 2 GiB allowed by Google Play Store (still limited to 4 GiB ZIP limit). 
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "APK Packaging", Meta = (DisplayName = "Allow large OBB files."))
	bool bAllowLargeOBBFiles;

	// If checked, UE4Game files will be placed in ExternalFilesDir which is removed on uninstall.
	// You should also check this if you need to save you game progress without requesting runtime WRITE_EXTERNAL_STORAGE permission in android api 23+
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "APK Packaging", Meta = (DisplayName = "Use ExternalFilesDir for UE4Game files?"))
	bool bUseExternalFilesDir;

	// The permitted orientation of the application on the device
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "APK Packaging")
	TEnumAsByte<EAndroidScreenOrientation::Type> Orientation;

	// Maximum supported aspect ratio (width / height). Android will automatically letterbox application on devices with bigger aspect ratio
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "APK Packaging", Meta = (DisplayName = "Maximum supported aspect ratio."))
	float MaxAspectRatio;

	// Level of verbosity to use during packaging with Ant
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "APK Packaging")
	TEnumAsByte<EAndroidAntVerbosity::Type> AntVerbosity;

	// Should the software navigation buttons be hidden or not
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "APK Packaging", Meta = (DisplayName = "Enable FullScreen Immersive on KitKat and above devices."))
	bool bFullScreen;

	UPROPERTY(GlobalConfig, EditAnywhere, Category = "APK Packaging", Meta = ( DisplayName = "Enable improved virtual keyboard"))
	bool bEnableNewKeyboard;
	
	// The preferred depth buffer bitcount for Android
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "APK Packaging", Meta = (DisplayName = "Preferred Depth Buffer format"))
	TEnumAsByte<EAndroidDepthBufferPreference::Type> DepthBufferPreference;

	// Verifies the device supports at least one of the cooked texture formats at runtime
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "APK Packaging", Meta = (DisplayName = "Validate texture formats"))
	bool bValidateTextureFormats;

	// Any extra tags for the <manifest> node
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Advanced APK Packaging", Meta = (DisplayName = "Extra Tags for <manifest> node"))
	TArray<FString> ExtraManifestNodeTags;

	// Any extra tags for the <application> node
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Advanced APK Packaging", Meta = (DisplayName = "Extra Tags for <application> node"))
	TArray<FString> ExtraApplicationNodeTags;

	// Any extra tags for the com.epicgames.UE4.GameActivity <activity> node
	// Any extra settings for the <application> section (an optional file <Project>/Build/Android/ManifestApplicationAdditions.txt will also be included)
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Advanced APK Packaging", Meta = (DisplayName = "Extra Settings for <application> section (\\n to separate lines)"))
	FString ExtraApplicationSettings;

	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Advanced APK Packaging", Meta = (DisplayName = "Extra Tags for UE4.GameActivity <activity> node"))
	TArray<FString> ExtraActivityNodeTags;

	// Any extra settings for the main <activity> section (an optional file <Project>/Build/Android/ManifestApplicationActivtyAdditions.txt will also be included)
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Advanced APK Packaging", Meta = (DisplayName = "Extra Settings for <activity> section (\\n to separate lines)"))
	FString ExtraActivitySettings;

	// Any extra permissions your app needs (an optional file <Project>/Build/Android/ManifestRequirementsAdditions.txt will also be included,
	// or an optional file <Project>/Build/Android/ManifestRequirementsOverride.txt will replace the entire <!-- Requirements --> section)
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Advanced APK Packaging", Meta = (DisplayName = "Extra Permissions (e.g. 'android.permission.INTERNET')"))
	TArray<FString> ExtraPermissions;

	// Add required permission to support Voice chat
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Advanced APK Packaging", Meta = (DisplayName = "Add permissions to support Voice chat (RECORD_AUDIO)"))
	bool bAndroidVoiceEnabled;

	// Configure AndroidManifest.xml for Oculus Mobile
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Advanced APK Packaging", Meta = (DisplayName = "Configure the AndroidManifest for deployment to Oculus Mobile"))
	bool bPackageForGearVR;

	// Removes Oculus Signature Files (osig) from APK if Gear VR APK signed for distribution and enables entitlement checker
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Advanced APK Packaging", Meta = (DisplayName = "Remove Oculus Signature Files from Distribution APK"))
	bool bRemoveOSIG;

	// Configure AndroidManifest.xml to support specific hardward configurations, position and orientation of the head and controller.
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Advanced APK Packaging", Meta = (DisplayName = "Configure GoogleVR to support specific hardware configurations"))
	TArray<TEnumAsByte<EGoogleVRCaps::Type>> GoogleVRCaps;

	// Configure the Android to run in sustained performance with lower max speeds, but no FPS fluctuations due to temperature
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Advanced APK Packaging", Meta = (DisplayName = "Configure GoogleVR for sustained-performance mode"))
	bool bGoogleVRSustainedPerformance;

	// This is the file that keytool outputs, specified with the -keystore parameter (file should be in <Project>/Build/Android)
	UPROPERTY(GlobalConfig, EditAnywhere, Category = DistributionSigning, Meta = (DisplayName = "Key Store (output of keytool, placed in <Project>/Build/Android)"))
	FString KeyStore;
	
	// This is the name of the key that you specified with the -alias parameter to keytool
	UPROPERTY(GlobalConfig, EditAnywhere, Category = DistributionSigning, Meta = (DisplayName = "Key Alias (-alias parameter to keytool)"))
	FString KeyAlias;
	
	// This is the password that you specified FOR THE KEYSTORE NOT THE KEY, when running keytool (either with -storepass or by typing it in).
	UPROPERTY(GlobalConfig, EditAnywhere, Category = DistributionSigning, Meta = (DisplayName = "Key Store Password (-storepass parameter to keytool)"))
	FString KeyStorePassword;
	
	// This is the password for the key that you may have specified with keytool, if it's different from the keystore password. Leave blank to use same as Keystore
	UPROPERTY(GlobalConfig, EditAnywhere, Category = DistributionSigning, Meta = (DisplayName = "Key Password (leave blank to use Key Store Password)"))
	FString KeyPassword;

	// Enable ArmV7 support? (this will be used if all type are unchecked)
	UPROPERTY(GlobalConfig, EditAnywhere, Category = Build, meta = (DisplayName = "Support armv7 [aka armeabi-v7a]"))
	bool bBuildForArmV7;

	// Enable Arm64 support?
	UPROPERTY(GlobalConfig, EditAnywhere, Category = Build, meta = (DisplayName = "Support arm64 [aka arm64-v8a]"))
	bool bBuildForArm64;

	// Enable x86-64 support? [CURRENTLY FOR FULL SOURCE GAMES ONLY]
	UPROPERTY(GlobalConfig, EditAnywhere, Category = Build, meta = (DisplayName = "Support x86_64 [aka x64]"))
	bool bBuildForX8664;

	// Enable ES2 support?
	UPROPERTY(GlobalConfig, EditAnywhere, Category = Build, meta = (DisplayName = "Support OpenGL ES2"))
	bool bBuildForES2;

	// Enable ES3.1 support?
	UPROPERTY(GlobalConfig, EditAnywhere, Category = Build, meta = (DisplayName = "Support OpenGL ES3.1"))
	bool bBuildForES31;

	// Enable Vulkan rendering support?
	UPROPERTY(GlobalConfig, EditAnywhere, Category = Build, meta = (DisplayName = "Support Vulkan"))
	bool bSupportsVulkan;

	// Whether to detect Vulkan device support by default, if the project is packaged with Vulkan support. If unchecked, the -detectvulkan commandline will enable Vulkan detection.
	UPROPERTY(GlobalConfig, EditAnywhere, AdvancedDisplay, Category = Build, meta = (DisplayName = "Detect Vulkan device support", EditCondition = "bSupportsVulkan"))
	bool bDetectVulkanByDefault;

	// Build the shipping config with hidden visibility by default. Results in smaller .so file but will also removes symbols used to display callstack dumps.
	UPROPERTY(GlobalConfig, EditAnywhere, Category = AdvancedBuild, meta = (DisplayName = "Build with hidden symbol visibility in shipping config. [Experimental]"))
	bool bBuildWithHiddenSymbolVisibility;

	// Always save .so file with symbols allowing use of addr2line on raw callstack addresses.
	UPROPERTY(GlobalConfig, EditAnywhere, Category = AdvancedBuild, meta = (DisplayName = "Always save a copy of the libUE4.so with symbols. [Experimental]"))
	bool bSaveSymbols;

	// If selected, the checked architectures will be split into separate .apk files [CURRENTLY FOR FULL SOURCE GAMES ONLY]
	// @todo android fat binary: Currently, there isn't much utility in merging multiple .so's into a single .apk except for debugging,
	// but we can't properly handle multiple GPU architectures in a single .apk, so we are disabling the feature for now
	// The user will need to manually select the apk to run in their Visual Studio debugger settings (see Override APK in TADP, for instance)
// 	UPROPERTY(GlobalConfig, EditAnywhere, Category = Build)
// 	bool bSplitIntoSeparateApks;

	// Should Google Play support be enabled?
	UPROPERTY(GlobalConfig, EditAnywhere, Category = GooglePlayServices)
	bool bEnableGooglePlaySupport;

	// Enabling this adds GET_ACCOUNTS to manifest and user must give permission.  Required for reset achievements.
	UPROPERTY(GlobalConfig, EditAnywhere, Category = GooglePlayServices, meta = (DisplayName = "Request Access Token On Connect"))
	bool bUseGetAccounts;

	// The app id obtained from the Google Play Developer Console
	UPROPERTY(GlobalConfig, EditAnywhere, Category = GooglePlayServices)
	FString GamesAppID;

	// Mapping of game achievement names to IDs generated by Google Play.
	UPROPERTY(GlobalConfig, EditAnywhere, Category = GooglePlayServices)
	TArray<FGooglePlayAchievementMapping> AchievementMap;

	// Mapping of game leaderboard names to IDs generated by Google Play.
	UPROPERTY(GlobalConfig, EditAnywhere, Category = GooglePlayServices)
	TArray<FGooglePlayLeaderboardMapping> LeaderboardMap;

	// Enabling this includes the AdMob SDK and will be detected by Google Play Console on upload of APK.  Disable if you do not need ads to remove this warning.
	UPROPERTY(GlobalConfig, EditAnywhere, Category = GooglePlayServices, meta = (DisplayName = "Include AdMob support for ads"))
	bool bSupportAdMob;

	// The unique identifier for the ad obtained from AdMob.
	UPROPERTY(GlobalConfig, EditAnywhere, Category = GooglePlayServices)
	FString AdMobAdUnitID;

	// Identifiers for ads obtained from AdMob
	UPROPERTY(GlobalConfig, EditAnywhere, Category = GooglePlayServices)
	TArray<FString> AdMobAdUnitIDs;

	// The unique identifier for this application (needed for IAP)
	UPROPERTY(GlobalConfig, EditAnywhere, Category = GooglePlayServices)
	FString GooglePlayLicenseKey;

	// The sender id obtained from Firebase Console, leave blank to disable (associate this with your app in Google Player Developer Console).
	UPROPERTY(GlobalConfig, EditAnywhere, Category = GooglePlayServices, meta = (DisplayName = "Google Cloud Messaging Sender ID"))
	FString GCMClientSenderID;

	/** Show the launch image as a startup slash screen */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = LaunchImages, meta = (DisplayName = "Show launch image"))
	bool bShowLaunchImage;

	/** Allows accelerometer, magnetometer, and gyroscope event handling, disabling may improve performance. */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = Input, meta = (DisplayName = "Allow IMU Sampling"))
	bool bAllowIMU;

	// If checked, Bluetooth connected controllers will send input
	UPROPERTY(GlobalConfig, EditAnywhere, Category = Input, meta = (DisplayName = "Allow Bluetooth controllers"))
	bool bAllowControllers;

	// If checked, controllers will not send Android_Back and Android_Menu events that might cause unnecce
	UPROPERTY(GlobalConfig, EditAnywhere, Category = Input, meta = (DisplayName = "Block Android system keys being sent from controllers"))
	bool bBlockAndroidKeysOnControllers;

	/** Android encoding options. */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = Audio, meta = (DisplayName = "Encoding Format"))
	TEnumAsByte<EAndroidAudio::Type> AndroidAudio;

	/** Sample rate to run the audio mixer with. */
	UPROPERTY(config, EditAnywhere, Category = "Audio", Meta = (DisplayName = "Audio Mixer Sample Rate"))
	int32 AudioSampleRate;

	/** The amount of audio to compute each callback block. Lower values decrease latency but may increase CPU cost. */
	UPROPERTY(config, EditAnywhere, Category = "Audio", meta = (ClampMin = "512", ClampMax = "4096", DisplayName = "Callback Buffer Size"))
	int32 AudioCallbackBufferFrameSize;

	/** The number of buffers to keep enqueued. More buffers increases latency, but can compensate for variable compute availability in audio callbacks on some platforms. */
	UPROPERTY(config, EditAnywhere, Category = "Audio", meta = (ClampMin = "2", UIMin = "2", DisplayName = "Number of Buffers To Enqueue"))
	int32 AudioNumBuffersToEnqueue;

	/** The max number of channels (voices) to limit for this platform. The max channels used will be the minimum of this value and the global audio quality settings. A value of 0 will not apply a platform channel count max. */
	UPROPERTY(config, EditAnywhere, Category = "Audio", meta = (ClampMin = "0", UIMin = "0", DisplayName = "Max Channels"))
	int32 AudioMaxChannels;

	/** The number of workers to use to compute source audio. Will only use up to the max number of sources. Will evenly divide sources to each source worker. */
	UPROPERTY(config, EditAnywhere, Category = "Audio", meta = (ClampMin = "0", UIMin = "0", DisplayName = "Number of Source Workers"))
	int32 AudioNumSourceWorkers;

	/** Which of the currently enabled spatialization plugins to use on Windows. */
	UPROPERTY(config, EditAnywhere, Category = "Audio")
	FString SpatializationPlugin;

	/** Which of the currently enabled reverb plugins to use on Windows. */
	UPROPERTY(config, EditAnywhere, Category = "Audio")
	FString ReverbPlugin;

	/** Which of the currently enabled occlusion plugins to use on Windows. */
	UPROPERTY(config, EditAnywhere, Category = "Audio")
	FString OcclusionPlugin;

	/** Various overrides for how this platform should handle compression and decompression */
	UPROPERTY(config, EditAnywhere, Category = "Audio")
	FPlatformRuntimeAudioCompressionOverrides CompressionOverrides;

	UPROPERTY(config, EditAnywhere, Category = "Audio|CookOverrides")
	bool bResampleForDevice;

	// Mapping of which sample rates are used for each sample rate quality for a specific platform.

	UPROPERTY(config, EditAnywhere, Category = "Audio|CookOverrides|ResamplingQuality", meta = (DisplayName = "Max"))
	float MaxSampleRate;

	UPROPERTY(config, EditAnywhere, Category = "Audio|CookOverrides|ResamplingQuality", meta = (DisplayName = "High"))
	float HighSampleRate;

	UPROPERTY(config, EditAnywhere, Category = "Audio|CookOverrides|ResamplingQuality", meta = (DisplayName = "Medium"))
	float MedSampleRate;

	UPROPERTY(config, EditAnywhere, Category = "Audio|CookOverrides|ResamplingQuality", meta = (DisplayName = "Low"))
	float LowSampleRate;

	UPROPERTY(config, EditAnywhere, Category = "Audio|CookOverrides|ResamplingQuality", meta = (DisplayName = "Min"))
	float MinSampleRate;

	// Scales all compression qualities when cooking to this platform. For example, 0.5 will halve all compression qualities, and 1.0 will leave them unchanged.
	UPROPERTY(config, EditAnywhere, Category = "Audio|CookOverrides")
	float CompressionQualityModifier;

	// When set to anything beyond 0, this will ensure any SoundWaves longer than this value, in seconds, to stream directly off of the disk.
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Audio|CookOverrides", meta = (DisplayName = "Stream All Soundwaves Longer Than: "))
	float AutoStreamingThreshold;

	// Several Android graphics debuggers require configuration changes to be made to your application in order to operate. Choosing an option from this menu will configure your project to work with that graphics debugger. 
	UPROPERTY(GlobalConfig, EditAnywhere, Category = GraphicsDebugger)
	TEnumAsByte<EAndroidGraphicsDebugger::Type> AndroidGraphicsDebugger;

	/** The path to your Mali Graphics Debugger installation (eg C:/Program Files/ARM/Mali Developer Tools/Mali Graphics Debugger v4.2.0) */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = GraphicsDebugger)
	FDirectoryPath MaliGraphicsDebuggerPath;

	/** Include ETC1 textures when packaging with the Android (Multi) variant. ETC1 will be included if no other formats are selected. */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = MultiTextureFormats, meta = (DisplayName = "Include ETC1 textures"))
	bool bMultiTargetFormat_ETC1;

	/** Include ETC1a textures when packaging with the Android (Multi) variant. */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = MultiTextureFormats, meta = (DisplayName = "Include ETC1 - alpha textures"))
	bool bMultiTargetFormat_ETC1a;

	/** Include ETC2 textures when packaging with the Android (Multi) variant. */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = MultiTextureFormats, meta = (DisplayName = "Include ETC2 textures"))
	bool bMultiTargetFormat_ETC2;

	/** Include DXT textures when packaging with the Android (Multi) variant. */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = MultiTextureFormats, meta = (DisplayName = "Include DXT textures"))
	bool bMultiTargetFormat_DXT;

	/** Include PVRTC textures when packaging with the Android (Multi) variant. */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = MultiTextureFormats, meta = (DisplayName = "Include PVRTC textures"))
	bool bMultiTargetFormat_PVRTC;

	/** Include ATC textures when packaging with the Android (Multi) variant. */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = MultiTextureFormats, meta = (DisplayName = "Include ATC textures"))
	bool bMultiTargetFormat_ATC;

	/** Include ASTC textures when packaging with the Android (Multi) variant. */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = MultiTextureFormats, meta = (DisplayName = "Include ASTC textures"))
	bool bMultiTargetFormat_ASTC;

	/** Priority for the ETC1 texture format when launching on device or packaging using Android_Multi. The highest priority format supported by the device will be used. Default value is 0.1. */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = TextureFormatPriorities, meta = (DisplayName = "ETC1 texture format priority"))
	float TextureFormatPriority_ETC1;

	/** Priority for the ETC1a texture format when launching on device or packaging using Android_Multi. The highest priority format supported by the device will be used. Default value is 0.2. */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = TextureFormatPriorities, meta = (DisplayName = "ETC1 - alpha texture format priority"))
	float TextureFormatPriority_ETC1a;

	/** Priority for the ETC2 texture format when launching on device or packaging using Android_Multi. The highest priority format supported by the device will be used. Default value is 0.2. */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = TextureFormatPriorities, meta = (DisplayName = "ETC2 texture format priority"))
	float TextureFormatPriority_ETC2;

	/** Priority for the DXT texture format when launching on device or packaging using Android_Multi. The highest priority format supported by the device will be used. Default value is 0.6. */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = TextureFormatPriorities, meta = (DisplayName = "DXT texture format priority"))
	float TextureFormatPriority_DXT;

	/** Priority for the PVRTC texture format when launching on device or packaging using Android_Multi. The highest priority format supported by the device will be used. Default value is 0.8. */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = TextureFormatPriorities, meta = (DisplayName = "PVRTC texture format priority"))
	float TextureFormatPriority_PVRTC;

	/** Priority for the ATC texture format when launching on device or packaging using Android_Multi. The highest priority format supported by the device will be used. Default value is 0.5. */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = TextureFormatPriorities, meta = (DisplayName = "ATC texture format priority"))
	float TextureFormatPriority_ATC;

	/** Priority for the ASTC texture format when launching on device or packaging using Android_Multi. The highest priority format supported by the device will be used. Default value is 0.9. */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = TextureFormatPriorities, meta = (DisplayName = "ASTC texture format priority"))
	float TextureFormatPriority_ASTC;

	// Which SDK to package and compile Java with (a specific version or (without quotes) 'latest' for latest version on disk, or 'matchndk' to match the NDK API Level)
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Project SDK Override", Meta = (DisplayName = "SDK API Level (specific version, 'latest', or 'matchndk' - see tooltip)"))
	FString SDKAPILevelOverride;

	// Which NDK to compile with (a specific version or (without quotes) 'latest' for latest version on disk). Note that choosing android-21 or later won't run on pre-5.0 devices.
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Project SDK Override", Meta = (DisplayName = "NDK API Level (specific version or 'latest' - see tooltip)"))
	FString NDKAPILevelOverride;

	virtual void PostReloadConfig(class UProperty* PropertyThatWasLoaded) override;

#if WITH_EDITOR
	// UObject interface
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostInitProperties() override;
	// End of UObject interface

private:
	void EnsureValidGPUArch();
#endif
};
