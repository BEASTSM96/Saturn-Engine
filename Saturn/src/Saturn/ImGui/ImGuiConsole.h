#pragma once

#include "Saturn/Core/Base.h"

#include <vector>
#include <string>

namespace Saturn {

	class ImGuiConsole
	{
	public:
		class Message
		{
		public:
			enum class Level : int8_t
			{
				Invalid = -1,
				Trace = 0,
				Debug = 1,
				Info = 2,
				Warn = 3,
				Error = 4,
				Critical = 5,
				Off = 6, // Display nothing
			};
		private:
			struct Color { float r, g, b, a; };
		public:
			Message(const std::string message = "", Level level = Level::Invalid);
			void OnImGuiRender( void );

			static Level GetLowerLevel(Level level);
			static Level GetHigherLevel(Level level);
			static const char* GetLevelName(Level level);
		private:
			static Color GetRenderColor(Level level);
		public:
			const std::string m_Message;
			const Level m_Level;
			static std::vector<Level> s_Levels;
		};
	public:
		~ImGuiConsole() = default;
		static void AddMessage(std::shared_ptr<Message> message);
		static void Clear( void );
		static void OnImGuiRender(bool* show);
	protected:
		ImGuiConsole() = default;
	private:
		struct ImGuiRendering
		{
			static void ImGuiRenderHeader( void );
			static void ImGuiRenderSettings( void );
			static void ImGuiRenderMessages( void );
		};
	private:
		static float s_DisplayScale;
		static uint16_t s_MessageBufferCapacity;
		static uint16_t s_MessageBufferSize;
		static uint16_t s_MessageBufferBegin;
		static Message::Level s_MessageBufferRenderFilter;
		static std::vector<std::shared_ptr<Message>> s_MessageBuffer;
		static bool s_AllowScrollingToBottom;
		static bool s_RequestScrollToBottom;
	};
}