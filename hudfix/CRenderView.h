#pragma once

namespace dice {
	class CRenderView {
	public:

		struct Mat4 {
			float m[4][4];
		};

		struct ViewPort;

		virtual int addRef(); //
		virtual int getRef(); //
		virtual int release(); //
		virtual void* queryInterface(unsigned int id); //
		virtual void function4(); //
		virtual ViewPort* getViewPort();
		virtual void function6(); //
		virtual void setFov(float); //
		virtual float getFov(); //
		virtual void function9(); //
		virtual void function10(); //
		virtual void function11(); //
		virtual void function12(); //
		virtual void function13(); //
		virtual void function14(); //
		virtual void function15(); //
		virtual void function16(); //
		virtual void function17(); //
		virtual void function18(); //
		virtual void function19(); //
		virtual void function20(); //
		virtual void function21(); //
		virtual void function22(); //
		virtual void function23(); //
		virtual void function24(); //
		virtual Mat4* getInvViewMatrix(); //
		virtual Mat4* getViewMatrix(); //
		virtual Mat4* getProjectionMatrix(); //
		virtual void function28(); //
		virtual Mat4* function29(); //
		virtual void function30(); //
		virtual void function31(); //
		virtual void function32(); //
		virtual Mat4* getViewProjMatrix(); //

		unsigned int refCount; // 0x004
		struct ViewPort {
			float x; //0x0008  
			float y; //0x000C  
			float width; //0x0010  
			float height; //0x0014
		} viewPort;
		int _0x18;
		float _0x1C;
		float _0x20;
		float fovAngle; // 0x24
		float zNear; // 0x28
		float zFar; // 0x2C
		float invAspectRatio; // 0x30
		float viewVolumeWidth; // 0x34
		float viewVolumeheight; // 0x38
		Mat4 invViewMatrix; // 0x3C
		Mat4 viewMatrix; // 0x7C
		Mat4 unknown; // 0xBC
		Mat4 viewProjMatrix; // 0xFC
		Mat4 projectionMatrix; // 0x13C
	};
}