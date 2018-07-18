
#include "stdafx.h"

//////////////////////////////////////////////////////////////////////////////////
// Helper Macros - use as you please

#define GETFLOAT(p) *((float*)&p)
#define GETBOOL(p) *((bool*)&p)
#define GETINT(p) p
#define GETSTRING(p) *((char**)&p)
#define GETFLOATARRAY(p) p ? ((float*)p+1) : 0
#define GETINTARRAY(p) p ? ((int*)p+1) : 0
#define GETSTRINGARRAY(p) p ? ((char**)p+1) : 0
#define GETARRAYSIZE(p) p ? *((int*)p) : 0
#define GETFRAME(p) p ? ((float*)p+1) : 0
#define GETFRAMESIZE(p) p ? *((int*)p) : 0
#define GETBITMAPWIDTH(p) p ? *((int*)p) : 0
#define GETBITMAPHEIGHT(p) p ? *((int*)p+1) : 0
#define GETBITMAPCHANNELS(p) p ? *((int*)p+2) : 0
#define GETBITMAPDATA(p) p ? ((BYTE*)p+12) : 0
#define GETBITMAPBYTES(p) p ? *((int*)p) * *((int*)p+1) * *((int*)p+2) : 0
#define NEWINTARRAY(p,n) if(n>0) { *((int**)&p)=new int[n+1]; ((int*)p)[0]=n; }
#define NEWFLOATARRAY(p,n) if(n>0) { *((float**)&p)=new float[n+1]; ((int*)p)[0]=n; }
#define NEWSTRINGARRAY(p,n) if(n>0) { *((char***)&p)=new char*[n+1]; ((int*)p)[0]=n; }
#define DELETESTRING(p) if(p) { delete *((char**)&p); p=0; }
#define DELETEINTARRAY(p) if(p) { delete *((int**)&p); p=0; }
#define DELETEFLOATARRAY(p) if(p) { delete *((float**)&p); p=0; }
#define DELETESTRINGARRAY(p) if(p) { for( int j=0; j<*((int*)p); j++ ) { if( ((char**)p+1)[j] ) delete ((char**)p+1)[j]; } delete *((char***)&p); p=0; }

//////////////////////////////////////////////////////////////////////////////////


// Changes alpha for a bitmap
// Inputs : bitmap, alpha value (0-1)
extern "C" __declspec(dllexport) void changeBitmap( int nParams, int* pIn, int* pOut )
{
	if( pIn && pOut && nParams >= 2 )
	{
		if( pIn[0] )
		{
			int w = GETBITMAPWIDTH(pIn[0]);
			int h = GETBITMAPHEIGHT(pIn[0]);
			int c = GETBITMAPCHANNELS(pIn[0]);

			int bytes = w*h*c;

			BYTE* pData = GETBITMAPDATA(pIn[0]);

			if( pOut[0] )
			{
				int wO = GETBITMAPWIDTH(pOut[0]);
				int hO = GETBITMAPHEIGHT(pOut[0]);
				int cO = GETBITMAPCHANNELS(pOut[0]);

				if( wO*hO*cO != bytes )
				{
					delete (BYTE*)pOut[0];
					pOut[0] = 0;
				}				
			}

			float alpha = GETFLOAT(pIn[1]);
			alpha = (alpha < 0.0f ? 0.0f : (alpha > 1.0f ? 1.0f: alpha));

			if( pData && bytes > 0 )
			{
				BYTE* pNewOut = (BYTE*)pOut[0];
				if( !pNewOut )
				{
					pNewOut = new BYTE[bytes+4*3];
					*((BYTE**)(&pOut[0])) = pNewOut;
				}

				memcpy(pNewOut,(BYTE*)pIn[0],bytes+4*3);

				BYTE* pData = GETBITMAPDATA(pOut[0]);

				if( c == 4 )
				{
					for( int i=0; i<w*h ; i++ )
					{
						pData[(i+1)*c-1] *= alpha;
					}
				}
			}

			GETINT(pOut[2]) = w;
			GETINT(pOut[3]) = h;
			GETINT(pOut[4]) = c;
		}
	}
}


// Audio echo delay
// Inputs : frame, delay (samples), feedback (0-1), mix (0-1), counter for buffer, buffer (frame)
extern "C" __declspec(dllexport) void echoDelay( int nParams, int* pIn, int* pOut )
{
	if( pIn && pOut && nParams >= 6 )
	{
		if( pIn[0] )
		{
			float* pData = GETFRAME(pOut[0]);
			int n = GETFRAMESIZE(pIn[0]);

			int delay = GETINT(pIn[1]);
			float feed = GETFLOAT(pIn[2]);
			float mix = GETFLOAT(pIn[3]);
			int ctr = GETINT(pOut[4]);

			float* pBuffer = GETFRAME(pIn[5]);
			int buffSize = GETFRAMESIZE(pIn[5]);

			if( pBuffer && buffSize >= delay && pData )
			{
				float curr;

				for( int i=0; i<n; i++ )
				{
					curr = pData[i];
					pData[i] = (mix)*pBuffer[ctr] + curr;

					pBuffer[ctr] = pBuffer[ctr]*feed + curr;
					ctr++;
					if( ctr >= delay ) ctr = 0;
				}

				GETINT(pOut[4]) = ctr;
			}
		}
	}
}

// Adds two floats together
// Inputs : float1, float2
extern "C" __declspec(dllexport) void addFloats( int nParams, int* pIn, int* pOut )
{
	if( pIn && pOut && nParams >= 2 )
	{
		float f0 = GETFLOAT(pIn[0]);
		float f1 = GETFLOAT(pIn[1]);
		
		GETFLOAT(pOut[0]) = f0+f1;
	}
}

// Makes an array of strings upppercase
// Inputs : array of strings
extern "C" __declspec(dllexport) void makeUppercaseArray( int nParams, int* pIn, int* pOut )
{
	if( pIn && pOut && nParams >= 1 )
	{
		char** pStringsIn = GETSTRINGARRAY(pIn[0]);
		int nIn = GETARRAYSIZE(pIn[0]);

		int nOut = GETARRAYSIZE(pOut[0]);

		DELETESTRINGARRAY(pOut[0]);

		if( nIn > 0 )
		{
			NEWSTRINGARRAY(pOut[0],nIn)

			char** pStringsOut = GETSTRINGARRAY(pOut[0]);

			for( int j=0; j<nIn; j++ )
			{
				char* strIn = pStringsIn[j];
				pStringsOut[j] = 0;

				if( strIn )
				{
					int len = strlen(strIn);

					if( len > 0 )
					{
						char* strOut = new char[len+1];

						int i=0;
						char c;

						while( strIn[i] )
						{
							c = strIn[i];
							strOut[i] = toupper(c);
							i++;
						}
						strOut[len]=0;
						pStringsOut[j] = strOut;
					}
				}
			}
		}
	}	
}

// Makes a string uppercase
// Inputs : string
extern "C" __declspec(dllexport) void makeUppercase( int nParams, int* pIn, int* pOut )
{
	if( pIn && pOut && nParams >= 1 )
	{

		char* strIn = GETSTRING(pIn[0]);

		DELETESTRING(pOut[0]);

		if( strIn )
		{
			int i=0;
			char c;

			int len = strlen(strIn);
			char* strOut = new char[len+1];

			while( strIn[i] )
			{
				c = strIn[i];
				strOut[i] = toupper(c);
				i++;
			}

			strOut[len]=0;
			GETSTRING(pOut[0]) = strOut;
		}
	}	
}

