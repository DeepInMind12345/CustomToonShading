// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Tools.DotNETCommon;

namespace UnrealBuildTool
{
	/// <summary>
	/// Contains information about a target required to deploy it. Written at build time, and read back in when UBT needs to run the deploy step.
	/// </summary>
	class UEBuildDeployTarget
	{
		/// <summary>
		/// Path to the project file
		/// </summary>
		public readonly FileReference ProjectFile;

		/// <summary>
		/// The name of this target
		/// </summary>
		public readonly string TargetName;

		/// <summary>
		/// Type of the target to build
		/// </summary>
		public readonly TargetType TargetType;

		/// <summary>
		/// The platform being built
		/// </summary>
		public readonly UnrealTargetPlatform Platform;

		/// <summary>
		/// The configuration being built
		/// </summary>
		public readonly UnrealTargetConfiguration Configuration;

		/// <summary>
		/// The project directory, or engine directory for targets without a project file.
		/// </summary>
		public readonly DirectoryReference ProjectDirectory;

		/// <summary>
		/// Path to the generated build receipt.
		/// </summary>
		public readonly FileReference BuildReceiptFileName;

		/// <summary>
		/// Construct the deployment info from a target
		/// </summary>
		/// <param name="Target">The target being built</param>
		public UEBuildDeployTarget(UEBuildTarget Target)
		{
			this.ProjectFile = Target.ProjectFile;
			this.TargetName = Target.TargetName;
			this.TargetType = Target.TargetType;
			this.Platform = Target.Platform;
			this.Configuration = Target.Configuration;
			this.ProjectDirectory = Target.ProjectDirectory;
			this.BuildReceiptFileName = Target.ReceiptFileName;
		}

		/// <summary>
		/// Read the deployment info from a file on disk
		/// </summary>
		/// <param name="Location">Path to the file to read</param>
		public UEBuildDeployTarget(FileReference Location)
		{
			using (BinaryReader Reader = new BinaryReader(File.Open(Location.FullName, FileMode.Open, FileAccess.Read, FileShare.Read)))
			{
				ProjectFile = Reader.ReadFileReference();
				TargetName = Reader.ReadString();
				TargetType = (TargetType)Reader.ReadInt32();
				Platform = (UnrealTargetPlatform)Reader.ReadInt32();
				Configuration = (UnrealTargetConfiguration)Reader.ReadInt32();
				ProjectDirectory = Reader.ReadDirectoryReference();
				BuildReceiptFileName = Reader.ReadFileReference();
			}
		}

		/// <summary>
		/// Write the deployment info to a file on disk
		/// </summary>
		/// <param name="Location">File to write to</param>
		public void Write(FileReference Location)
		{
			DirectoryReference.CreateDirectory(Location.Directory);
			using (BinaryWriter Writer = new BinaryWriter(File.Open(Location.FullName, FileMode.Create, FileAccess.Write, FileShare.Read)))
			{
				Writer.Write(ProjectFile);
				Writer.Write(TargetName);
				Writer.Write((Int32)TargetType);
				Writer.Write((Int32)Platform);
				Writer.Write((Int32)Configuration);
				Writer.Write(ProjectDirectory);
				Writer.Write(BuildReceiptFileName);
			}
		}
	}
}
