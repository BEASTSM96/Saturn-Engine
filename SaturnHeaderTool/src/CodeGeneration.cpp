/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2024 BEAST                                                           *
*                                                                                           *
* Permission is hereby granted, free of charge, to any person obtaining a copy              *
* of this software and associated documentation files (the "Software"), to deal             *
* in the Software without restriction, including without limitation the rights              *
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell                 *
* copies of the Software, and to permit persons to whom the Software is                     *
* furnished to do so, subject to the following conditions:                                  *
*                                                                                           *
* The above copyright notice and this permission notice shall be included in all            *
* copies or substantial portions of the Software.                                           *
*                                                                                           *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR                *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,                  *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE               *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER                    *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,             *
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE             *
* SOFTWARE.                                                                                 *
*********************************************************************************************
*/

#include "CodeGeneration.h"

#include "Saturn/GameFramework/SClass.h"

#include <regex>
#include <fstream>

namespace Saturn {

	HeaderTool::HeaderTool()
	{

	}

	HeaderTool::~HeaderTool()
	{
		m_Commands.clear();
	}

	void HeaderTool::SetWorkingDir( const std::filesystem::path& rPath )
	{
		m_WorkingDir = rPath;
	}

	void HeaderTool::SubmitWorkList( const std::vector<std::filesystem::path>& rCommands )
	{
		m_Commands.reserve( rCommands.size() );

		for( const auto& rCommand : rCommands )
		{
			HeaderToolCommand command;
			command.Filepath = rCommand;

			m_Commands.push_back( command );
		}
	}

	void HeaderTool::StartGeneration()
	{
		for( auto& rCommand : m_Commands )
		{
			// Stage 1: Generate Header file.
			GenerateHeader( rCommand );

			// Stage 2: Generate Source file.
			GenerateSource( rCommand );
		}
	}

	static bool LineIsNotComment( const std::string& rLine )
	{
		std::regex regex( R"(^\s*(//.*|/\*.*\*/|/\*.*))" );
		return !std::regex_match( rLine, regex );
	}

	static std::pair<std::string, std::string> GetClassNameAndBaseClass( const std::string& rLine )
	{
		if( !LineIsNotComment( rLine ) )
			return {};

		std::regex classPattern( R"(class\s+(\w+)\s*:\s*public\s+(\w+))" );
		std::smatch match;

		if( std::regex_search( rLine, match, classPattern ) )
		{
			return std::make_pair( match[ 1 ].str(), match[ 2 ].str() );
		}

		return {};
	}

	static SPropertyType StringToSPropertyType( const std::string& rStr ) { return SPropertyType::Unknown; }
	static std::string SPropertyTypeToString( SPropertyType type ) { return "Saturn::SPropertyType::Unknown"; }

	bool HeaderTool::GenerateHeader( HeaderToolCommand& rCommand ) 
	{
		bool result = true;
		
		std::filesystem::path outputPath = m_WorkingDir;
		outputPath /= rCommand.Filepath.stem();
		outputPath += ".Gen.h";

		std::ofstream fout( outputPath );

		fout << "/* Generated code, DO NOT modify! */\n";
		fout << "#pragma once\n";
		fout << "#include \"Saturn/GameFramework/Core/GameScript.h\"\n\n";

		// Read the header file
		std::ifstream headerFile( rCommand.Filepath );

		bool LastLineHadSP = false, SClassFound = false, GeneratedBodyFound = false;

		std::string line;
		int lineNumber = 0;
		while( std::getline( headerFile, line ) )
		{
			lineNumber++;
			if( line.empty() ) continue;

			if( rCommand.ClassName.empty() || rCommand.BaseClass.empty() )
			{
				auto pair = GetClassNameAndBaseClass( line );

				rCommand.ClassName = pair.first;
				rCommand.BaseClass = pair.second;
			}

			std::smatch match;
			std::regex typeRegex( R"((\w+)\s+(\w+))" );

			if( LastLineHadSP && LineIsNotComment( line ) )
			{
				if( std::regex_search( line, match, typeRegex ) )
				{
					const std::string type = match[ 1 ].str();
					const std::string name = match[ 2 ].str();

					SPropertyType realType = StringToSPropertyType( type );

					SProperty p{ name, type, realType };
					rCommand.Properties[ lineNumber ] = p;
				}
				else
				{
					//std::cout << rPath.string() << ":error (CG003) | Expected variable definition after SPROPERTY macro.\n";
				}

				LastLineHadSP = false;
			}

			// Check for SPROPERTY
			std::regex spropertyRegex( R"(SPROPERTY\((.*)\))", std::regex::extended );

			if( std::regex_search( line, match, spropertyRegex ) && LineIsNotComment( line ) )
			{
				const std::string args = match[ 1 ].str();
				const std::string remainingContent = line.substr( match.position() + match.length() ).c_str();

				if( remainingContent.empty() || remainingContent[ 0 ] == '\n' )
				{
					LastLineHadSP = true;
				}
				else
				{
					if( std::regex_search( line, match, std::regex( R"(SPROPERTY\(.*?\)\s+(\w+)\s+(\w+))", std::regex::extended ) ) )
					{
						const std::string type = match[ 1 ].str();
						const std::string name = match[ 2 ].str();

						SPropertyType spropType = StringToSPropertyType( type );

						SProperty p{ .Name = name, .NativeType = type, .Type = spropType };
						rCommand.Properties[ lineNumber ] = p;
					}
					else
					{
						//std::cout << rPath.string() << ":error (CG003) | Expected variable definition after SPROPERTY macro.\n";
					}
				}
			}

			if( !SClassFound )
			{
				if( std::regex_search( line, match, std::regex( R"(SCLASS\((.*)\))", std::regex::extended ) ) && LineIsNotComment( line ) )
				{
					const std::string args = match[ 1 ].str();

					if( args.contains( "Spawnable" ) )
					{
						rCommand.ClassFlags |= ( uint32_t ) SClassFlags::Spawnable;
					}

					if( args.contains( "VisibleInEditor" ) )
					{
						rCommand.ClassFlags |= ( uint32_t ) SClassFlags::VisibleInEditor;
					}

					if( args.contains( "NoMetadata" ) )
					{
						rCommand.ClassFlags |= ( uint32_t ) SClassFlags::NoMetadata;
					}

					SClassFound = true;
				}
			}

			if( !GeneratedBodyFound )
			{
				if( std::regex_search( line, match, std::regex( R"(GENERATED_BODY\((.*)\))", std::regex::extended ) ) && LineIsNotComment( line ) )
				{
					const std::string args = match[ 1 ].str();

					if( !args.empty() )
					{
						//std::cout << rPath.string() << ":warning (CG002A) | No arguments are allowed in the GENERATED_BODY macro, arguments omitted.\n";
					}

					// Parse generated header

					std::string baseFileId = std::format( "FID_{0}_h_{1}", rCommand.ClassName, lineNumber );

					std::string CFI = std::format( "#undef CURRENT_FILE_ID\n#define CURRENT_FILE_ID FID_{0}_h\n\n", rCommand.ClassName );

					fout << CFI;

					std::string idGeneratedBody = std::format( "#define {0}_GENERATED_BODY {0}_CLASSDECLS\n", baseFileId );
					std::string classDecls = std::format( "#define {0}_CLASSDECLS \\\nprivate: \\\n\tSAT_DECLARE_CLASS({1},{2}) \\\npublic:\\\n\n", baseFileId, rCommand.ClassName, rCommand.BaseClass );

					fout << classDecls;
					fout << idGeneratedBody;

					GeneratedBodyFound = true;
				}
			}
		}

		headerFile.close();
		fout.close();

		return result;
	}

	bool HeaderTool::GenerateSource( HeaderToolCommand& rCommand )
	{
		bool result = true;

		std::filesystem::path outputPath = m_WorkingDir;
		outputPath /= rCommand.Filepath.stem();
		outputPath += ".Gen.cpp";

		std::filesystem::path generatedHeaderPath = m_WorkingDir;
		generatedHeaderPath /= rCommand.Filepath.stem();
		generatedHeaderPath += ".Gen.h";

		std::ofstream fout( outputPath );

		fout << "/* Generated code, DO NOT modify! */\n";
		fout << "#include \"Saturn/GameFramework/Core/GameScript.h\"\n";
		fout << "#include \"Saturn/GameFramework/Core/ClassMetadataHandler.h\"\n";
		fout << "#include \"Saturn/Scene/Entity.h\"\n";
		fout << std::format( "#include \"{0}\"\n", rCommand.Filepath.string() );
		fout << std::format( "#include \"{0}\"\n\n", generatedHeaderPath.string() );

		auto& rClassName = rCommand.ClassName;

		fout << "extern \"C\" {\n";

		if( ( rCommand.ClassFlags & ( uint32_t ) SClassFlags::Spawnable ) != 0 )
		{
			fout << std::format( "__declspec(dllexport) Saturn::Entity* _Z_Create_{0}(Saturn::Scene* pScene)\n", rClassName );
			fout << "{\n";
			fout << std::format( "\tSaturn::Ref<{0}> Target = Saturn::Ref<{0}>::Create();\n", rClassName );
			fout << "\tSaturn::Ref<Saturn::Entity> TargetReturn = Target.As<Saturn::Entity>();\n";
			fout << "\treturn TargetReturn.Get();\n";
			fout << "}\n";

			fout << "//^^^ Spawnable\n";
		}
		else
		{
			fout << std::format( "__declspec(dllexport) Saturn::Entity* _Z_Create_{0}(Saturn::Scene* pScene)\n", rClassName );
			fout << "{\n";
			fout << "\treturn nullptr;\n";
			fout << "}\n";

			fout << "//^^^ NO Spawnable\n";
		}

		fout << "}\n\n";

		if( ( rCommand.ClassFlags & ( uint32_t ) SClassFlags::NoMetadata ) == 0 )
		{
			std::string realPath = generatedHeaderPath.string();

			size_t pos = 0;
			while( ( pos = realPath.find( '\\', pos ) ) != std::string::npos )
			{
				realPath.replace( pos, 1, "\\\\" );
				pos += 2;
			}

			fout << std::format( "static void ReflCreateMetadatFor_{0}()\n", rClassName );
			fout << "{\n";
			fout << std::format( "\tSaturn::SClassMetadata __Metadata_{0};\n", rClassName );
			fout << std::format( "\t__Metadata_{0}.Name = \"{0}\";\n", rClassName );
			fout << std::format( "\t__Metadata_{0}.ParentClassName = \"{1}\";\n", rClassName, rCommand.BaseClass );
			fout << std::format( "\t__Metadata_{0}.GeneratedSourcePath = __FILE__;\n", rClassName );
			fout << std::format( "\t__Metadata_{0}.HeaderPath = \"{1}\";\n", rClassName, realPath );
			fout << std::format( "\t__Metadata_{0}.ExternalData = true;\n", rClassName );
			fout << std::format( "\tSaturn::ClassMetadataHandler::Get().AddMetadata( __Metadata_{0} );\n", rClassName );
			fout << "}\n\n";
		}
		else
		{
			fout << std::format( "static void ReflCreateMetadatFor_{0}()\n", rClassName );
			fout << "{\n";
			fout << std::format( "\tSaturn::SClassMetadata __Metadata_{0};\n", rClassName );
			fout << std::format( "\t__Metadata_{0}.Name = \"{0}\";\n", rClassName );
			fout << std::format( "\tSaturn::ClassMetadataHandler::Get().AddMetadata( __Metadata_{0} );\n", rClassName );
			fout << "}\n\n";
		}

		std::string internalClassName = std::format( "{0}Int", rClassName );

		fout << "class " << internalClassName << "\n";
		fout << "{\n";
		fout << "public:\n";

		for( const auto& [lineNumber, rProperty] : rCommand.Properties )
		{
			// Set property function
			fout << "\tstatic void Set" << rProperty.Name << "( " << rClassName << "* pClass, " << rProperty.NativeType << " value )\n";
			fout << "\t{\n";
			fout << "\t\tpClass->" << rProperty.Name << " = value;\n";
			fout << "\t}\n";

			// Get property function
			fout << "\tstatic " << rProperty.NativeType << " Get" << rProperty.Name << "( " << rClassName << "* pClass )\n";
			fout << "\t{\n";
			fout << "\t\treturn pClass->" << rProperty.Name << ";\n";
			fout << "\t}\n";

			fout << "\n";
		}

		fout << "};\n\n";

		fout << "static void ReflRegisterPropetiesFor_" << rClassName << "()\n";
		fout << "{\n";

		for( const auto& [lineNumber, rProperty] : rCommand.Properties )
		{
			std::string stringType = SPropertyTypeToString( rProperty.Type );

			fout << std::format( "\tSaturn::SProperty Prop_{0} = {{ .Name = \"{0}\", .Type = {1}, .Flags = {2} }};\n", rProperty.Name, stringType, "0" );
			fout << std::format( "\tProp_{0}.pGetPropertyFunction = &{1}::Get{0};\n", rProperty.Name, internalClassName );
			fout << std::format( "\tProp_{0}.pSetPropertyFunction = &{1}::Set{0};\n", rProperty.Name, internalClassName );

			fout << std::format( "\tSaturn::ClassMetadataHandler::Get().RegisterProperty( \"{0}\", Prop_{1} );\n", internalClassName, rProperty.Name );
		}

		fout << "}\n\n";

		// Auto-Registration (DLL only).
		fout << std::format( "struct Ar{0}_RTEditor\n", rClassName );
		fout << "{\n";
		fout << std::format( "\tAr{0}_RTEditor()\n", rClassName );
		fout << "\t{\n";
		fout << std::format( "\t\tReflCreateMetadataFor_{0}();\n", rClassName );
		fout << std::format( "\t\tReflRegisterPropetiesFor_{0}();\n", rClassName );
		fout << "\t}\n";
		fout << "}\n\n";
		fout << std::format( "static Ar{0}_RTEditor Ar{0}_Runtime;\n", rClassName );
		fout << "//^^^ Auto-Registration\n";

		fout.close();

		return result;
	}
}