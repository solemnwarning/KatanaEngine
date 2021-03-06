/*	Copyright (C) 2011-2016 OldTimes Software

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "engine_base.h"

#include "video.h"

/*
	This document acts as a layer between the video sub-system
	and OpenGL. All OpenGL functionality should be here.
*/

/*===========================
	OPENGL ERROR HANDLING
===========================*/

/*	Checks glGetError and returns a generic string describing the fault.
*/
char *VideoLayer_GetErrorMessage(unsigned int uiGLError)
{
	VIDEO_FUNCTION_START
	switch (uiGLError)
	{
	case GL_NO_ERROR:
		return "No error has been recorded.";
	case GL_INVALID_ENUM:
		return "An unacceptable value is specified for an enumerated argument.";
	case GL_INVALID_VALUE:
		return "A numeric argument is out of range.";
	case GL_INVALID_OPERATION:
		return "The specified operation is not allowed in the current state.";
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		return "The framebuffer object is not complete.";
	case GL_OUT_OF_MEMORY:
		return "There is not enough memory left to execute the command.";
	case GL_STACK_UNDERFLOW:
		return "An attempt has been made to perform an operation that would	cause an internal stack to underflow.";
	case GL_STACK_OVERFLOW:
		return "An attempt has been made to perform an operation that would	cause an internal stack to overflow.";
	default:
		return "An unknown error occured.";
	}
	VIDEO_FUNCTION_END
}

/*===========================
	OPENGL MATRICES
===========================*/

static bool vl_matrixpushed = false;

void VideoLayer_PushMatrix(void)
{
	VIDEO_FUNCTION_START
	if (vl_matrixpushed)
		return;

	glPushMatrix();
	vl_matrixpushed = true;
	VIDEO_FUNCTION_END
}

void VideoLayer_PopMatrix(void)
{
	VIDEO_FUNCTION_START
	if (!vl_matrixpushed)
		return;
	
	glPopMatrix();
	vl_matrixpushed = false;
	VIDEO_FUNCTION_END
}

/*===========================
	OPENGL SHADERS
===========================*/

void VideoLayer_UseProgram(unsigned int program)
{
	if (program == Video.current_program)
		return;
	glUseProgram(program);
	Video.current_program = program;
}

/*===========================
	OPENGL TEXTURES
===========================*/

unsigned int VideoLayer_TranslateFormat(VideoTextureFormat_t Format)
{
	switch (Format)
	{
	case VIDEO_TEXTURE_FORMAT_BGR:
		return GL_BGR;
	case VIDEO_TEXTURE_FORMAT_BGRA:
		return GL_BGRA;
	case VIDEO_TEXTURE_FORMAT_LUMINANCE:
		return GL_LUMINANCE;
	case VIDEO_TEXTURE_FORMAT_RGB:
		return GL_RGB;
	case VIDEO_TEXTURE_FORMAT_RGBA:
		return GL_RGBA;
	default:
		Sys_Error("Unknown texture format! (%i)\n", Format);
	}

	// Won't be hit but meh, compiler will complain otherwise.
	return 0;
}

void VideoLayer_SetupTexture(VideoTextureFormat_t InternalFormat, VideoTextureFormat_t Format, unsigned int Width, unsigned int Height)
{
	VIDEO_FUNCTION_START
	glTexImage2D(GL_TEXTURE_2D, 0, 
		VideoLayer_TranslateFormat(InternalFormat),
		Width, Height, 0, 
		VideoLayer_TranslateFormat(Format), 
		GL_UNSIGNED_BYTE, NULL);
	VIDEO_FUNCTION_END
}

/*	TODO:
		Modify this so it works as a replacement for TexMgr_SetFilterModes.
*/
void VideoLayer_SetTextureFilter(VideoTextureFilter_t FilterMode)
{
	VIDEO_FUNCTION_START
	unsigned int SetFilter = 0;

	switch (FilterMode)
	{
	case VIDEO_TEXTURE_FILTER_LINEAR:
		SetFilter = GL_LINEAR;
		break;
	case VIDEO_TEXTURE_FILTER_NEAREST:
		SetFilter = GL_NEAREST;
		break;
	default:
		Sys_Error("Unknown texture filter type! (%i)\n", FilterMode);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, SetFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, SetFilter);
	VIDEO_FUNCTION_END
}

int VideoLayer_TranslateTextureEnvironmentMode(VideoTextureEnvironmentMode_t TextureEnvironmentMode)
{
	VIDEO_FUNCTION_START
	switch (TextureEnvironmentMode)
	{
	case VIDEO_TEXTURE_MODE_ADD:
		return GL_ADD;
	case VIDEO_TEXTURE_MODE_MODULATE:
		return GL_MODULATE;
	case VIDEO_TEXTURE_MODE_DECAL:
		return GL_DECAL;
	case VIDEO_TEXTURE_MODE_BLEND:
		return GL_BLEND;
	case VIDEO_TEXTURE_MODE_REPLACE:
		return GL_REPLACE;
	case VIDEO_TEXTURE_MODE_COMBINE:
		return GL_COMBINE;
	default:
		Sys_Error("Unknown texture environment mode! (%i)\n", TextureEnvironmentMode);
	}

	// Won't be hit but meh, compiler will complain otherwise.
	return 0;
	VIDEO_FUNCTION_END
}

void VideoLayer_SetTextureEnvironmentMode(VideoTextureEnvironmentMode_t TextureEnvironmentMode)
{
	VIDEO_FUNCTION_START
	// Ensure there's actually been a change.
	if (Video.textureunits[Video.current_textureunit].current_envmode == TextureEnvironmentMode)
		return;

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, 
		VideoLayer_TranslateTextureEnvironmentMode(TextureEnvironmentMode));

	Video.textureunits[Video.current_textureunit].current_envmode = TextureEnvironmentMode;
	VIDEO_FUNCTION_END
}

/*===========================
	OPENGL CAPABILITIES
===========================*/

typedef struct
{
	unsigned int uiFirst, uiSecond;

	const char *ccIdentifier;
} VideoLayerCapabilities_t;

VideoLayerCapabilities_t capabilities[] =
{
	{ VIDEO_ALPHA_TEST, GL_ALPHA_TEST, "ALPHA_TEST" },
	{ VIDEO_BLEND, GL_BLEND, "BLEND" },
	{ VIDEO_DEPTH_TEST, GL_DEPTH_TEST, "DEPTH_TEST" },
	{ VIDEO_TEXTURE_2D, GL_TEXTURE_2D, "TEXTURE_2D" },
	{ VIDEO_TEXTURE_GEN_S, GL_TEXTURE_GEN_S, "TEXTURE_GEN_S" },
	{ VIDEO_TEXTURE_GEN_T, GL_TEXTURE_GEN_T, "TEXTURE_GEN_T" },
	{ VIDEO_CULL_FACE, GL_CULL_FACE, "CULL_FACE" },
	{ VIDEO_STENCIL_TEST, GL_STENCIL_TEST, "STENCIL_TEST" },
	{ VIDEO_NORMALIZE, GL_NORMALIZE, "NORMALIZE" },
	{ VIDEO_MULTISAMPLE, GL_MULTISAMPLE, "MULTISAMPLE" },

	{ 0 }
};

/*	Enables video capabilities.
*/
void VideoLayer_Enable(unsigned int uiCapabilities)
{
	VIDEO_FUNCTION_START
	int i;
	for (i = 0; i < sizeof(capabilities); i++)
	{
		// Check if we reached the end of the list yet.
		if (!capabilities[i].uiFirst)
			break;

		if (uiCapabilities & VIDEO_TEXTURE_2D)
			Video.textureunit_state[Video.current_textureunit] = true;

		if (uiCapabilities & capabilities[i].uiFirst)
		{
			if (Video.debug_frame)
				plWriteLog(VIDEO_LOG, "Enabling %s (%i)\n", capabilities[i].ccIdentifier, Video.current_textureunit);

			glEnable(capabilities[i].uiSecond);
		}
	}
	VIDEO_FUNCTION_END
}

void VideoLayer_Disable(unsigned int uiCapabilities)
{
	VIDEO_FUNCTION_START
	int i;
	for (i = 0; i < sizeof(capabilities); i++)
	{
		// Check if we reached the end of the list yet.
		if (!capabilities[i].uiFirst)
			break;

		if (uiCapabilities & VIDEO_TEXTURE_2D)
			Video.textureunit_state[Video.current_textureunit] = false;

		if (uiCapabilities & capabilities[i].uiFirst)
		{
			// TODO: Implement debugging support.

			glDisable(capabilities[i].uiSecond);
		}
	}
	VIDEO_FUNCTION_END
}

/*	TODO: Want more control over the dynamics of this...
*/
void VideoLayer_BlendFunc(VideoBlend_t modea, VideoBlend_t modeb)
{
	VIDEO_FUNCTION_START
	glBlendFunc(modea, modeb);
	if (Video.debug_frame)
		plWriteLog(VIDEO_LOG, "Video: Setting blend mode (%i) (%i)\n", modea, modeb);
	VIDEO_FUNCTION_END
}

/*	Enable or disable writing into the depth buffer.
*/
void VideoLayer_DepthMask(bool mode)
{
	VIDEO_FUNCTION_START
	static bool cur_state = true;
	if (mode == cur_state) return;
	glDepthMask(mode);
	cur_state = mode;
	VIDEO_FUNCTION_END
}

/*===========================
	OPENGL BUFFERS
===========================*/

// RENDER BUFFER OBJECTS

void VideoLayer_GenerateRenderBuffer(unsigned int *buffer)
{
	VIDEO_FUNCTION_START
	glGenRenderbuffers(1, buffer);
	VIDEO_FUNCTION_END
}

void VideoLayer_DeleteRenderBuffer(unsigned int *buffer)
{
	VIDEO_FUNCTION_START
	glDeleteRenderbuffers(1, buffer);
	VIDEO_FUNCTION_END
}

void VideoLayer_BindRenderBuffer(unsigned int buffer)
{
	VIDEO_FUNCTION_START
	glBindRenderbuffer(GL_RENDERBUFFER, buffer);
	VIDEO_FUNCTION_END
}

void VideoLayer_RenderBufferStorage(int format, int samples, unsigned int width, unsigned int height)
{
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, format, width, height);

	// Ensure there weren't any issues.
	unsigned int glerror = glGetError();
	if (glerror != GL_NO_ERROR)
	{
		char *errorstring = "";
		switch (glerror)
		{
		case GL_INVALID_ENUM:
			errorstring = "Invalid internal format!";
			break;
		case GL_OUT_OF_MEMORY:
			errorstring = "Unable to create a data store of the requested size!";
			break;
		default:
			// This should *NEVER* occur.
			break;
		}
		Sys_Error("%s\n%s", VideoLayer_GetErrorMessage(glerror), errorstring);
	}
}

// VERTEX BUFFER OBJECTS

/*	Generates a single OpenGL buffer.
	glGenBuffers
*/
void VideoLayer_GenerateVertexBuffer(unsigned int *uiBuffer) 
{
	VIDEO_FUNCTION_START
	glGenBuffers(1, uiBuffer);
	VIDEO_FUNCTION_END
}

/*	Deletes a single OpenGL buffer.
	glDeleteBuffers
*/
void VideoLayer_DeleteVertexBuffer(unsigned int *uiBuffer)
{
	VIDEO_FUNCTION_START
	glDeleteBuffers(1, uiBuffer);
	VIDEO_FUNCTION_END
}

// FRAME BUFFER OBJECTS

void VideoLayer_ClearStencilBuffer(void)
{
	glClear(GL_STENCIL_BUFFER_BIT);
}

/*	Generates a single framebuffer.
	glGenFramebuffers
*/
void VideoLayer_GenerateFrameBuffer(unsigned int *uiBuffer) 
{
	VIDEO_FUNCTION_START
	glGenFramebuffers(1, uiBuffer);
	VIDEO_FUNCTION_END
}

/*	Ensures that the framebuffer is valid, otherwise throws an error.
	glCheckFramebufferStatus
*/
void VideoLayer_CheckFrameBufferStatus()
{
	int status;
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status == GL_FRAMEBUFFER_COMPLETE)
		return;

	// Generic GL error.
	if (status == 0)
		Sys_Error("An error occured when checking framebuffer! (%s)\n", VideoLayer_GetErrorMessage(glGetError()));

	switch (status)
	{
	case GL_FRAMEBUFFER_UNDEFINED:
		Sys_Error("Default framebuffer doesn't exist!\n");
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		Sys_Error("Framebuffer attachment points are incomplete!\n");
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		Sys_Error("Framebuffer doesn't have an image attached!\n");
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		Sys_Error("Invalid attachment type!\n");
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		Sys_Error("INCOMPLETE READ BUFFER\n");
	case GL_FRAMEBUFFER_UNSUPPORTED:
		Sys_Error("UNSUPPORTED\n");
	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
		Sys_Error("INCOMPLETE MULTISAMPLE\n");
	case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_ARB:
		Sys_Error("INCOMPLETE LAYER TARGETS\n");
	}

	Sys_Error("An unknown error occured when checking framebuffer status!\n");
}

/*	Binds the given framebuffer.
	glBindFramebuffer
*/
void VideoLayer_BindFrameBuffer(VideoFBOTarget_t vtTarget, unsigned int uiBuffer)
{
	VIDEO_FUNCTION_START
	unsigned int outtarget;

	// TODO: Get these named up so we can easily debug.
	switch (vtTarget)
	{
	case VIDEO_FBO_DRAW:
		outtarget = GL_DRAW_BUFFER;
		break;
	case VIDEO_FBO_READ:
		outtarget = GL_READ_BUFFER;
		break;
	default:
		outtarget = GL_FRAMEBUFFER;
	}

	glBindFramebuffer(outtarget, uiBuffer);

	// Ensure there weren't any issues.
	unsigned int glerror = glGetError();
	if (glerror != GL_NO_ERROR)
	{
		char *errorstring = "";
		switch (glerror)
		{
		case GL_INVALID_ENUM:
			errorstring = "Invalid framebuffer target!";
			break;
		case GL_INVALID_OPERATION:
			errorstring = "Invalid framebuffer object!";
			break;
		default:
			// This should *NEVER* occur.
			break;
		}
		Sys_Error("%s\n%s", VideoLayer_GetErrorMessage(glerror), errorstring);
	}
	VIDEO_FUNCTION_END
}

/*	Deletes the given framebuffer.
	glDeleteFramebuffers
*/
void VideoLayer_DeleteFrameBuffer(unsigned int *uiBuffer)
{
	VIDEO_FUNCTION_START
	glDeleteFramebuffers(1, uiBuffer);
	VIDEO_FUNCTION_END
}

void VideoLayer_AttachFrameBufferRenderBuffer(unsigned int attachment, unsigned int buffer)
{
	VIDEO_FUNCTION_START
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, buffer);
	VIDEO_FUNCTION_END
}

void VideoLayer_AttachFrameBufferTexture(gltexture_t *buffer)
{
	VIDEO_FUNCTION_START
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer->texnum, 0);
	VIDEO_FUNCTION_END
}

/*===========================
	OPENGL DRAWING
===========================*/

typedef struct
{
	VideoPrimitive_t vpPrimitive;

	unsigned int uiGL;

	const char *ccIdentifier;
} VideoPrimitives_t;

VideoPrimitives_t vpVideoPrimitiveList[] =
{
	{ VIDEO_PRIMITIVE_LINE, GL_LINES, "LINES" },
	{ VIDEO_PRIMITIVE_TRIANGLES, GL_TRIANGLES, "TRIANGLES" },
	{ VIDEO_PRIMITIVE_TRIANGLE_FAN, GL_TRIANGLE_FAN, "TRIANGLE_FAN" },
	{ VIDEO_PRIMITIVE_TRIANGLE_FAN_LINE, GL_LINES, "TRIANGLE_FAN_LINE" },
	{ VIDEO_PRIMITIVE_TRIANGLE_STRIP, GL_TRIANGLE_STRIP, "TRIANGLE_STRIP" }
};

unsigned int VideoLayer_TranslatePrimitiveType(const VideoPrimitive_t primitive)
{
	int	i;
	for (i = 0; i < sizeof(vpVideoPrimitiveList); i++)
		if (primitive == vpVideoPrimitiveList[i].vpPrimitive)
			return vpVideoPrimitiveList[i].uiGL;

	return VIDEO_PRIMITIVE_IGNORE;
}

/*	Deals with tris view and different primitive types, then finally draws
	the given arrays.
*/
void VideoLayer_DrawArrays(const VideoPrimitive_t vpPrimitiveType, unsigned int uiSize, bool bWireframe)
{
	unsigned int uiPrimitiveType = VideoLayer_TranslatePrimitiveType(vpPrimitiveType);
	if (uiPrimitiveType == VIDEO_PRIMITIVE_IGNORE)
		Sys_Error("Invalid primitive type! (%i)\n", vpPrimitiveType);

	if (bWireframe)
	{
		switch (vpPrimitiveType)
		{
		case VIDEO_PRIMITIVE_LINE:
		case VIDEO_PRIMITIVE_TRIANGLES:
			uiPrimitiveType = GL_LINES;
			break;
		default:
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
	}

	glDrawArrays(uiPrimitiveType, 0, uiSize);

	if (bWireframe)
		if ((vpPrimitiveType != VIDEO_PRIMITIVE_LINE) &&
			(vpPrimitiveType != VIDEO_PRIMITIVE_TRIANGLES))
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}