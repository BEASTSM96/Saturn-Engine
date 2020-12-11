#pragma once

#define SAT_USE_OLD_KEY_CODES

#ifdef SAT_USE_OLD_KEY_CODES
	//From GLFW
	/* Printable keys */
	#define SAT_KEY_SPACE              32
	#define SAT_KEY_APOSTROPHE         39  /* ' */
	#define SAT_KEY_COMMA              44  /* , */
	#define SAT_KEY_MINUS              45  /* - */
	#define SAT_KEY_PERIOD             46  /* . */
	#define SAT_KEY_SLASH              47  /* / */
	#define SAT_KEY_0                  48
	#define SAT_KEY_1                  49
	#define SAT_KEY_2                  50
	#define SAT_KEY_3                  51
	#define SAT_KEY_4                  52
	#define SAT_KEY_5                  53
	#define SAT_KEY_6                  54
	#define SAT_KEY_7                  55
	#define SAT_KEY_8                  56
	#define SAT_KEY_9                  57
	#define SAT_KEY_SEMICOLON          59  /* ; */
	#define SAT_KEY_EQUAL              61  /* = */
	#define SAT_KEY_A                  65
	#define SAT_KEY_B                  66
	#define SAT_KEY_C                  67
	#define SAT_KEY_D                  68
	#define SAT_KEY_E                  69
	#define SAT_KEY_F                  70
	#define SAT_KEY_G                  71
	#define SAT_KEY_H                  72
	#define SAT_KEY_I                  73
	#define SAT_KEY_J                  74
	#define SAT_KEY_K                  75
	#define SAT_KEY_L                  76
	#define SAT_KEY_M                  77
	#define SAT_KEY_N                  78
	#define SAT_KEY_O                  79
	#define SAT_KEY_P                  80
	#define SAT_KEY_Q                  81
	#define SAT_KEY_R                  82
	#define SAT_KEY_S                  83
	#define SAT_KEY_T                  84
	#define SAT_KEY_U                  85
	#define SAT_KEY_V                  86
	#define SAT_KEY_W                  87
	#define SAT_KEY_X                  88
	#define SAT_KEY_Y                  89
	#define SAT_KEY_Z                  90
	#define SAT_KEY_LEFT_BRACKET       91  /* [ */
	#define SAT_KEY_BACKSLASH          92  /* \ */
	#define SAT_KEY_RIGHT_BRACKET      93  /* ] */
	#define SAT_KEY_GRAVE_ACCENT       96  /* ` */
	#define SAT_KEY_WORLD_1            161 /* non-US #1 */
	#define SAT_KEY_WORLD_2            162 /* non-US #2 */

	/* Function keys */
	#define SAT_KEY_ESCAPE             256
	#define SAT_KEY_ENTER              257
	#define SAT_KEY_TAB                258
	#define SAT_KEY_BACKSPACE          259
	#define SAT_KEY_INSERT             260
	#define SAT_KEY_DELETE             261
	#define SAT_KEY_RIGHT              262
	#define SAT_KEY_LEFT               263
	#define SAT_KEY_DOWN               264
	#define SAT_KEY_UP                 265
	#define SAT_KEY_PAGE_UP            266
	#define SAT_KEY_PAGE_DOWN          267
	#define SAT_KEY_HOME               268
	#define SAT_KEY_END                269
	#define SAT_KEY_CAPS_LOCK          280
	#define SAT_KEY_SCROLL_LOCK        281
	#define SAT_KEY_NUM_LOCK           282
	#define SAT_KEY_PRINT_SCREEN       283
	#define SAT_KEY_PAUSE              284
	#define SAT_KEY_F1                 290
	#define SAT_KEY_F2                 291
	#define SAT_KEY_F3                 292
	#define SAT_KEY_F4                 293
	#define SAT_KEY_F5                 294
	#define SAT_KEY_F6                 295
	#define SAT_KEY_F7                 296
	#define SAT_KEY_F8                 297
	#define SAT_KEY_F9                 298
	#define SAT_KEY_F10                299
	#define SAT_KEY_F11                300
	#define SAT_KEY_F12                301
	#define SAT_KEY_F13                302
	#define SAT_KEY_F14                303
	#define SAT_KEY_F15                304
	#define SAT_KEY_F16                305
	#define SAT_KEY_F17                306
	#define SAT_KEY_F18                307
	#define SAT_KEY_F19                308
	#define SAT_KEY_F20                309
	#define SAT_KEY_F21                310
	#define SAT_KEY_F22                311
	#define SAT_KEY_F23                312
	#define SAT_KEY_F24                313
	#define SAT_KEY_F25                314
	#define SAT_KEY_KP_0               320
	#define SAT_KEY_KP_1               321
	#define SAT_KEY_KP_2               322
	#define SAT_KEY_KP_3               323
	#define SAT_KEY_KP_4               324
	#define SAT_KEY_KP_5               325
	#define SAT_KEY_KP_6               326
	#define SAT_KEY_KP_7               327
	#define SAT_KEY_KP_8               328
	#define SAT_KEY_KP_9               329
	#define SAT_KEY_KP_DECIMAL         330
	#define SAT_KEY_KP_DIVIDE          331
	#define SAT_KEY_KP_MULTIPLY        332
	#define SAT_KEY_KP_SUBTRACT        333
	#define SAT_KEY_KP_ADD             334
	#define SAT_KEY_KP_ENTER           335
	#define SAT_KEY_KP_EQUAL           336
	#define SAT_KEY_LEFT_SHIFT         340
	#define SAT_KEY_LEFT_CONTROL       341
	#define SAT_KEY_LEFT_ALT           342
	#define SAT_KEY_LEFT_SUPER         343
	#define SAT_KEY_RIGHT_SHIFT        344
	#define SAT_KEY_RIGHT_CONTROL      345
	#define SAT_KEY_RIGHT_ALT          346
	#define SAT_KEY_RIGHT_SUPER        347
	#define SAT_KEY_MENU               348

#endif


#define SAT_USE_NEW_KEY_CODES
#ifdef SAT_USE_NEW_KEY_CODES

namespace Saturn {

	using KeyCode = uint16_t;

	namespace Key {

		enum : KeyCode {
			// From glfw3.h
			Space               = 32,
			Apostrophe          = 39, /* ' */
			Comma               = 44, /* , */
			Minus               = 45, /* - */
			Period              = 46, /* . */
			Slash               = 47, /* / */

			D0                  = 48, /* 0 */
			D1                  = 49, /* 1 */
			D2                  = 50, /* 2 */
			D3                  = 51, /* 3 */
			D4                  = 52, /* 4 */
			D5                  = 53, /* 5 */
			D6                  = 54, /* 6 */
			D7                  = 55, /* 7 */
			D8                  = 56, /* 8 */
			D9                  = 57, /* 9 */

			Semicolon           = 59, /* ; */
			Equal               = 61, /* = */

			A                   = 65,
			B                   = 66,
			C                   = 67,
			D                   = 68,
			E                   = 69,
			F                   = 70,
			G                   = 71,
			H                   = 72,
			I                   = 73,
			J                   = 74,
			K                   = 75,
			L                   = 76,
			M                   = 77,
			N                   = 78,
			O                   = 79,
			P                   = 80,
			Q                   = 81,
			R                   = 82,
			S                   = 83,
			T                   = 84,
			U                   = 85,
			V                   = 86,
			W                   = 87,
			X                   = 88,
			Y                   = 89,
			Z                   = 90,

			LeftBracket         = 91,  /* [ */
			Backslash           = 92,  /* \ */
			RightBracket        = 93,  /* ] */
			GraveAccent         = 96,  /* ` */

			World1              = 161, /* non-US #1 */
			World2              = 162, /* non-US #2 */

			/* Function keys */
			Escape              = 256,
			Enter               = 257,
			Tab                 = 258,
			Backspace           = 259,
			Insert              = 260,
			Delete              = 261,
			Right               = 262,
			Left                = 263,
			Down                = 264,
			Up                  = 265,
			PageUp              = 266,
			PageDown            = 267,
			Home                = 268,
			End                 = 269,
			CapsLock            = 280,
			ScrollLock          = 281,
			NumLock             = 282,
			PrintScreen         = 283,
			Pause               = 284,
			F1                  = 290,
			F2                  = 291,
			F3                  = 292,
			F4                  = 293,
			F5                  = 294,
			F6                  = 295,
			F7                  = 296,
			F8                  = 297,
			F9                  = 298,
			F10                 = 299,
			F11                 = 300,
			F12                 = 301,
			F13                 = 302,
			F14                 = 303,
			F15                 = 304,
			F16                 = 305,
			F17                 = 306,
			F18                 = 307,
			F19                 = 308,
			F20                 = 309,
			F21                 = 310,
			F22                 = 311,
			F23                 = 312,
			F24                 = 313,
			F25                 = 314,

			/* Keypad */
			KP0                 = 320,
			KP1                 = 321,
			KP2                 = 322,
			KP3                 = 323,
			KP4                 = 324,
			KP5                 = 325,
			KP6                 = 326,
			KP7                 = 327,
			KP8                 = 328,
			KP9                 = 329,
			KPDecimal           = 330,
			KPDivide            = 331,
			KPMultiply          = 332,
			KPSubtract          = 333,
			KPAdd               = 334,
			KPEnter             = 335,
			KPEqual             = 336,

			LeftShift           = 340,
			LeftControl         = 341,
			LeftAlt             = 342,
			LeftSuper           = 343,
			RightShift          = 344,
			RightControl        = 345,
			RightAlt            = 346,
			RightSuper          = 347,
			Menu                = 348
		};

	}
}


#endif // SAT_USE_NEW_KEY_CODES
