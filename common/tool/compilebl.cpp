

#include "compilebl.h"
#include "../save/edmap.h"
#include "../utils.h"
#include "../platform.h"
#include "../texture.h"
#include "../save/save.h"
#include "../save/saveedm.h"


//#define COMPILEB_DEBUG

TexFitRow::TexFitRow()
{
	bounds[0].x = 0;
	bounds[0].y = 0;
	bounds[1].x = 0;
	bounds[1].y = 0;
}

TexFitInfo::TexFitInfo()
{
	tiletimes.x = 0;
	tiletimes.y = 0;
}

// get a power of 2 number that is big enough to hold 'lowerbound' but does not exceed 2048
int Max2Pow(int lowerbound)
{
	int twopow = 2;

	while( twopow < lowerbound && twopow < 2048 )
		twopow *= 2;

	return twopow;
}

void AllocTex(LoadedTex* empty, int width, int height, int channels)
{
	empty->data = (unsigned char*)malloc(width * height * channels * sizeof(unsigned char));
	empty->sizeX = width;
	empty->sizeY = height;
	empty->channels = channels;
}

// determine how many times a triangle tiles on a texture
Vec2i TileTimes(Vec2f* tc)
{
	Vec2f tc0 = tc[0];
	Vec2f tc1 = tc[1];
	Vec2f tc2 = tc[2];

	float minu = min(tc0.x, min(tc1.x, tc2.x));
	float minv = min(tc0.y, min(tc1.y, tc2.y));
	float maxu = max(tc0.x, max(tc1.x, tc2.x));
	float maxv = max(tc0.y, max(tc1.y, tc2.y));

	float rangeu = maxu - minu;
	float rangev = maxv - minv;

	float offu = minu - floor(minu);
	float offv = minv - floor(minv);

	float ceilrangeu = ceil(rangeu + offu);
	float ceilrangev = ceil(rangev + offv);

	/*
	float rangeu = maxu - minu;
	float rangev = maxv - minv;

	float offu = minu - floor(minu);
	float offv = minv - floor(minv);
	*/

	Vec2i tiletimes(ceilrangeu, ceilrangev);

#ifdef COMPILEB_DEBUG
	if(tiletimes.x == 11 || tiletimes.y == 11)
	{
		g_log<<"--------------11 or 11 tile times---------------"<<endl;
		g_log<<"ceilrange = "<<ceilrangeu<<","<<ceilrangev<<endl;
		g_log<<"range = "<<rangeu<<","<<rangev<<endl;
		g_log<<"min = "<<minu<<","<<minv<<endl;
		g_log<<"max = "<<maxu<<","<<maxv<<endl;
		g_log<<"[0] = "<<tc0.x<<","<<tc0.y<<endl;
		g_log<<"[1] = "<<tc1.x<<","<<tc1.y<<endl;
		g_log<<"[2] = "<<tc2.x<<","<<tc2.y<<endl;
		g_log<<"--------------11 or 11 tile times---------------"<<endl;
		g_log.flush();
	}
#endif

	return tiletimes;
}

// determine how many times a collection of triangles tiles on a texture
Vec2i TileTimes(VertexArray* va)
{
	Vec2i maxtile(0,0);

	for(int j=0; j<va->numverts; j+=3)
	{
		Vec2i tritile = TileTimes(&va->texcoords[j]);

		if(tritile.x > maxtile.x)
			maxtile.x = tritile.x;
		if(tritile.y > maxtile.y)
			maxtile.y = tritile.y;
	}

	return maxtile;
}

void Resample(LoadedTex* original, LoadedTex* empty, Vec2i newdim)
{
#ifdef COMPILEB_DEBUG
	g_log<<"resample...?"<<endl;
	g_log.flush();
#endif

	if(original == NULL || original->data == NULL || original->sizeX <= 0 || original->sizeY <= 0)
	{
#ifdef COMPILEB_DEBUG
		g_log<<"resample NULL 1"<<endl;
		g_log.flush();
#endif

		empty->data = NULL;

#ifdef COMPILEB_DEBUG
		g_log<<"resample NULL 2"<<endl;
		g_log.flush();
#endif

		empty->sizeX = 0;
		empty->sizeY = 0;

		if(original != NULL)
			empty->channels = original->channels;

		return;
	}

#ifdef COMPILEB_DEBUG
	g_log<<"resample "<<original->sizeX<<","<<original->sizeY<<" to "<<newdim.x<<","<<newdim.y<<endl;
	g_log.flush();
#endif

	AllocTex(empty, newdim.x, newdim.y, original->channels);

	double scaleW =  (double)newdim.x / (double)original->sizeX;
	double scaleH = (double)newdim.y / (double)original->sizeY;

	for(int cy = 0; cy < newdim.y; cy++)
	{
		for(int cx = 0; cx < newdim.x; cx++)
		{
			int pixel = cy * (newdim.x * original->channels) + cx*original->channels;
			int nearestMatch =  (int)(cy / scaleH) * original->sizeX * original->channels + (int)(cx / scaleW) * original->channels;

			empty->data[pixel    ] =  original->data[nearestMatch    ];
			empty->data[pixel + 1] =  original->data[nearestMatch + 1];
			empty->data[pixel + 2] =  original->data[nearestMatch + 2];

			if(original->channels > 3)
				empty->data[pixel + 3] =  original->data[nearestMatch + 3];
		}
	}

#ifdef COMPILEB_DEBUG
	g_log<<"\t done resample"<<endl;
	g_log.flush();
#endif
}

void Blit(LoadedTex* src, LoadedTex* dest, Vec2i pos)
{
	if(src == NULL || src->data == NULL)
		return;

	for(int x=0; x<src->sizeX; x++)
	{
		if(x+pos.x >= dest->sizeX)
			continue;

		for(int y=0; y<src->sizeY; y++)
		{
			if(y+pos.y >= dest->sizeY)
				continue;

			int srcpixel = x*src->channels + y*src->channels*src->sizeX;
			int destpixel = (x+pos.x)*dest->channels + (y+pos.y)*dest->channels*dest->sizeX;

			dest->data[destpixel + 0] = src->data[srcpixel + 0];
			dest->data[destpixel + 1] = src->data[srcpixel + 1];
			dest->data[destpixel + 2] = src->data[srcpixel + 2];

			if(dest->channels > 3)
			{
				if(src->channels <= 3)
					dest->data[destpixel + 3] = 255;
				else
					dest->data[destpixel + 3] = src->data[srcpixel + 3];
			}
		}
	}
}

void SaveJPEG(const char* fullpath, LoadedTex* image, float quality)
{
	FILE *outfile;
	if ((outfile = fopen(fullpath, "wb")) == NULL) 
	{
		return;
	}

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr       jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, outfile);

	cinfo.image_width      = image->sizeX;
	cinfo.image_height     = image->sizeY;
	cinfo.input_components = 3;
	cinfo.in_color_space   = JCS_RGB;

	jpeg_set_defaults(&cinfo);
	/*set the quality [0..100]  */
	jpeg_set_quality (&cinfo, 100*quality, true);
	jpeg_start_compress(&cinfo, true);

	JSAMPROW row_pointer;
	int row_stride = image->sizeX * 3;

	while (cinfo.next_scanline < cinfo.image_height)
	{
		row_pointer = (JSAMPROW) &image->data[cinfo.next_scanline*row_stride];
		jpeg_write_scanlines(&cinfo, &row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);

	fclose(outfile);

	jpeg_destroy_compress(&cinfo);
}


int SavePNG(const char* fullpath, LoadedTex* image)
{
	FILE *fp;
	png_structp png_ptr;
	png_infop info_ptr;
	//png_colorp palette;

	/* Open the file */
	fp = fopen(fullpath, "wb");
	if (fp == NULL)
		return (ERROR);

	/* Create and initialize the png_struct with the desired error handler
	* functions.  If you want to use the default stderr and longjump method,
	* you can supply NULL for the last three parameters.  We also check that
	* the library version is compatible with the one used at compile time,
	* in case we are using dynamically linked libraries.  REQUIRED.
	*/
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
		(png_voidp) NULL, NULL, NULL);

	if (png_ptr == NULL)
	{
		fclose(fp);
		return (ERROR);
	}

	/* Allocate/initialize the image information data.  REQUIRED */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		fclose(fp);
		png_destroy_write_struct(&png_ptr,  NULL);
		return (ERROR);
	}

	int color_type = PNG_COLOR_TYPE_RGB;

	if(image->channels == 4)
		color_type = PNG_COLOR_TYPE_RGBA;

	png_set_IHDR(png_ptr, info_ptr, image->sizeX, image->sizeY, 8, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	/* Set error handling.  REQUIRED if you aren't supplying your own
	* error handling functions in the png_create_write_struct() call.
	*/
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		/* If we get here, we had a problem writing the file */
		fclose(fp);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return (ERROR);
	}

	/* One of the following I/O initialization functions is REQUIRED */

	/* Set up the output control if you are using standard C streams */
	png_init_io(png_ptr, fp);


	png_bytep* row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * image->sizeY);
	for (int y=0; y<image->sizeY; y++)
		row_pointers[y] = (png_byte*)&image->data[y*image->sizeX*image->channels];

	png_write_info(png_ptr, info_ptr);
	png_write_image(png_ptr, row_pointers);
	png_write_end(png_ptr, NULL);

	//for (y=0; y<image->sizeY; y++)
	//   free(row_pointers[y]);
	free(row_pointers);


	/* This is the easy way.  Use it if you already have all the
	* image info living in the structure.  You could "|" many
	* PNG_TRANSFORM flags into the png_transforms integer here.
	*/
	//png_write_png(png_ptr, info_ptr, NULL, NULL);

	/* If you png_malloced a palette, free it here (don't free info_ptr->palette,
	* as recommended in versions 1.0.5m and earlier of this example; if
	* libpng mallocs info_ptr->palette, libpng will free it).  If you
	* allocated it with malloc() instead of png_malloc(), use free() instead
	* of png_free().
	*/
	//png_free(png_ptr, palette);
	//palette = NULL;

	/* Similarly, if you png_malloced any data that you passed in with
	* png_set_something(), such as a hist or trans array, free it here,
	* when you can be sure that libpng is through with it.
	*/
	//png_free(png_ptr, trans);
	//trans = NULL;
	/* Whenever you use png_free() it is a good idea to set the pointer to
	* NULL in case your application inadvertently tries to png_free() it
	* again.  When png_free() sees a NULL it returns without action, thus
	* avoiding the double-free security problem.
	*/

	/* Clean up after the write, and free any memory allocated */
	png_destroy_write_struct(&png_ptr, &info_ptr);

	/* Close the file */
	fclose(fp);

	/* That's it */
	return (1);
}

static bool transparency = false;
static unsigned int notexindex;
static int ntris = 0;
static list<unsigned int> uniquetexs;
static TexFitInfo texfitinfo[TEXTURES];

void CountTrisModel(list<ModelHolder> &modelholders)
{
	for(auto mhiter = modelholders.begin(); mhiter != modelholders.end(); mhiter++)
	{
		Model* pmodel = &g_model[mhiter->model];

		VertexArray* pva = &pmodel->m_va[0];

		ntris += pva->numverts / 3;
	}
}

void ListUniqueTexsModel(list<ModelHolder> &modelholders)
{
	for(auto mhiter = modelholders.begin(); mhiter != modelholders.end(); mhiter++)
	{
		Model* pmodel = &g_model[mhiter->model];

		VertexArray* pva = &pmodel->m_va[0];

		bool found = false;

		auto ut=uniquetexs.begin();
		for(; ut!=uniquetexs.end(); ut++)
		{
			TexFitInfo* tfi = &texfitinfo[ *ut ];

			if(*ut == pmodel->m_diffusem)
			{
				Vec2i tiletimes = TileTimes(pva);

				if(tiletimes.x > tfi->tiletimes.x)
					tfi->tiletimes.x = tiletimes.x;
				if(tiletimes.y > tfi->tiletimes.y)
					tfi->tiletimes.y = tiletimes.y;

				found = true;
				break;
			}
		}

		if(!found)
		{
			uniquetexs.push_back(pmodel->m_diffusem);

			TexFitInfo* tfi = &texfitinfo[ pmodel->m_diffusem ];
			Vec2i tiletimes = TileTimes(pva);
			TexFitInfo newtfi;
			tfi->tiletimes.x = tiletimes.x;
			tfi->tiletimes.y = tiletimes.y;

			if(g_texture[ pmodel->m_diffusem ].transp)
				transparency = true;
		}
	}
}

void CountTrisBrush(EdMap* map)
{
	for(auto br=map->m_brush.begin(); br!=map->m_brush.end(); br++)
	{
		for(int i=0; i<br->m_nsides; i++)
		{
			BrushSide* s = &br->m_sides[i];

			if(s->m_diffusem == notexindex)
				continue;

			VertexArray* va = &s->m_drawva;
			ntris += va->numverts / 3;
		}
	}
}

void ListUniqueTexsBrush(EdMap* map)
{
	for(auto br=map->m_brush.begin(); br!=map->m_brush.end(); br++)
	{
		for(int i=0; i<br->m_nsides; i++)
		{
			BrushSide* s = &br->m_sides[i];

			if(s->m_diffusem == notexindex)
				continue;

			VertexArray* va = &s->m_drawva;

			bool found = false;

			auto ut=uniquetexs.begin();
			for(; ut!=uniquetexs.end(); ut++)
			{
				TexFitInfo* tfi = &texfitinfo[ *ut ];

				if(*ut == s->m_diffusem)
				{
					Vec2i tiletimes = TileTimes(va);

					if(tiletimes.x > tfi->tiletimes.x)
						tfi->tiletimes.x = tiletimes.x;
					if(tiletimes.y > tfi->tiletimes.y)
						tfi->tiletimes.y = tiletimes.y;

					found = true;
					break;
				}
			}

			if(!found)
			{
				uniquetexs.push_back(s->m_diffusem);

				TexFitInfo* tfi = &texfitinfo[ s->m_diffusem ];
				Vec2i tiletimes = TileTimes(va);
				TexFitInfo newtfi;
				tfi->tiletimes.x = tiletimes.x;
				tfi->tiletimes.y = tiletimes.y;

				if(g_texture[ s->m_diffusem ].transp)
					transparency = true;
			}
		}
	}
}

static VertexArray fullva;

void AllocFinalVerts(EdMap* map, list<ModelHolder> &modelholders)
{
	fullva.free();
	fullva.alloc(ntris*3);

	int vindex = 0;

	for(auto br=map->m_brush.begin(); br!=map->m_brush.end(); br++)
	{
		for(int i=0; i<br->m_nsides; i++)
		{
			BrushSide* s = &br->m_sides[i];

			if(s->m_diffusem == notexindex)
				continue;

			VertexArray* va = &s->m_drawva;

			for(int j=0; j<va->numverts; j++)
			{
				fullva.vertices[vindex] = va->vertices[j];
				fullva.normals[vindex] = va->normals[j];
				vindex++;
			}
		}
	}

	for(auto mhiter = modelholders.begin(); mhiter != modelholders.end(); mhiter++)
	{
		VertexArray* pva = &mhiter->frames[0];

		for(int j=0; j<pva->numverts; j++)
		{
			fullva.vertices[vindex] = pva->vertices[j] + mhiter->translation;
			fullva.normals[vindex] = pva->normals[j];
			vindex++;
		}
	}
}

static TexRef texref[TEXTURES];
static LoadedTex** images;

void LoadAllRGBData()
{
	images = new LoadedTex*[uniquetexs.size()*3];

	int i=0;
	for(auto ut=uniquetexs.begin(); ut!=uniquetexs.end(); ut++, i++)
	{
		char fulldiffusepath[MAX_PATH+1];
		Texture* diffusetex = &g_texture[ *ut ];
		FullPath(diffusetex->filepath, fulldiffusepath);
		images[i] = LoadTexture(fulldiffusepath);
		TexRef* tr = &texref[ *ut ];
#ifdef COMPILEB_DEBUG
		tr->filepath = diffusetex->filepath;
#endif
		tr->diffindex = i;

#ifdef COMPILEB_DEBUG
		TexFitInfo* tfi = &texfitinfo[ *ut ];
		g_log<<"tile times "<<texref[ *ut ].filepath<<" "<<tfi->tiletimes.x<<","<<tfi->tiletimes.y<<endl;
		g_log.flush();
#endif

		i++;

		char basepath[MAX_PATH+1];
		strcpy(basepath, fulldiffusepath);
		StripExtension(basepath);

		char fullspecularpath[MAX_PATH+1];
		SpecPath(basepath, fullspecularpath);
		images[i] = LoadTexture(fullspecularpath);
		tr->specindex = i;

		i++;

		char fullnormalpath[MAX_PATH+1];
		NormPath(basepath, fullnormalpath);
		images[i] = LoadTexture(fullnormalpath);
		tr->normindex = i;

#ifdef COMPILEB_DEBUG
		g_log<<"------"<<endl;
		if(images[tr->diffindex])
			g_log<<fulldiffusepath<<" ("<<images[tr->diffindex]->sizeX<<","<<images[tr->diffindex]->sizeY<<")"<<endl;
		if(images[tr->specindex])
			g_log<<fullspecularpath<<" ("<<images[tr->specindex]->sizeX<<","<<images[tr->specindex]->sizeY<<")"<<endl;
		if(images[tr->normindex])
			g_log<<fullnormalpath<<" ("<<images[tr->normindex]->sizeX<<","<<images[tr->normindex]->sizeY<<")"<<endl;
		g_log<<i<<endl;
		g_log<<"------"<<endl;
		g_log.flush();
#endif
	}
}

static list<unsigned int> heightsorted;

void HeightSort()
{
	heightsorted.clear();
	int nextadd;
	int lastheight = 0;

	for(int i=0; i<uniquetexs.size(); i++)
	{
		nextadd = -1;
		lastheight = 0;

		int j=0;
		auto ut=uniquetexs.begin();
		for(; ut!=uniquetexs.end(); ut++, j++)
		{
			bool found = false;

			for(auto hs=heightsorted.begin(); hs!=heightsorted.end(); hs++)
			{
				if(*hs == *ut)
				{
					found = true;
					break;
				}
			}

			if(found)
				continue;

			TexRef* tr = &texref[ *ut ];
			LoadedTex* lt = images[ tr->diffindex ];
			TexFitInfo* tfi = &texfitinfo[ *ut ];
			int thisheight = 0;
			if(lt != NULL)        // image might not have loaded
				thisheight = lt->sizeY * tfi->tiletimes.y;

			if(thisheight >= lastheight || nextadd < 0)
			{
#ifdef COMPILEB_DEBUG
				g_log<<thisheight<<" >= "<<lastheight<<endl;
				g_log.flush();
#endif

				lastheight = thisheight;
				nextadd = (int)*ut;
			}
		}

		if(nextadd < 0)
			continue;        //shouldn't happen

		heightsorted.push_back((unsigned int)nextadd);
	}
}

static Vec2i maxdim(0, 0);
static list<TexFitRow> rows;

void FitImages()
{
	maxdim = Vec2i(0,0);	//Max combined image dimensions
	rows.clear();

	for(auto hs=heightsorted.begin(); hs!=heightsorted.end(); hs++)
	{
		TexRef* tr = &texref[ *hs ];
		TexFitInfo* tfi = &texfitinfo[ *hs ];

		LoadedTex* lt = images[ tr->diffindex ];

#ifdef COMPILEB_DEBUG
		g_log<<"------height sorted-----"<<endl;
		g_log<<"\t "<<tr->filepath<<endl;
		g_log<<"\t ("<<lt->sizeX<<","<<lt->sizeY<<")"<<endl;
		g_log<<"------height sorted-----"<<endl;
		g_log.flush();
#endif

		//if(maxdim.x < lt->sizeX)
		//        maxdim.x = lt->sizeX;
		//if(maxdim.y < lt->sizeY)
		//        maxdim.y = lt->sizeY;

		// Add first image row.
		if(rows.size() <= 0)
		{
			rows.push_back(TexFitRow());
			TexFitRow* firstrow = &*rows.begin();
			TexFit fit;
			fit.bounds[0].x = 0;
			fit.bounds[0].y = 0;
			fit.bounds[1].x = lt->sizeX * tfi->tiletimes.x;
			fit.bounds[1].y = lt->sizeY * tfi->tiletimes.y;
			fit.texindex = *hs;
			memcpy(tfi->bounds, fit.bounds, sizeof(Vec2i)*2);

			firstrow->fits.push_back(fit);
			firstrow->bounds[0].x = 0;
			firstrow->bounds[0].y = 0;
			firstrow->bounds[1].x = lt->sizeX * tfi->tiletimes.x;
			firstrow->bounds[1].y = lt->sizeY * tfi->tiletimes.y;

			maxdim.x += lt->sizeX * tfi->tiletimes.x;
			maxdim.y += lt->sizeY * tfi->tiletimes.y;

#ifdef COMPILEB_DEBUG
			g_log<<"\tfit1 to "<<fit.bounds[0].x<<","<<fit.bounds[0].y<<"->"<<fit.bounds[1].x<<","<<fit.bounds[1].y<<endl;
			g_log.flush();
#endif

			continue;
		}

		TexFitRow* addto = NULL;
		int leftest = 0;

		for(auto row=rows.begin(); row!=rows.end(); row++)
		{
			// If addto == NULL, adding it to the first row will be high enough.
			// Otherwise, we have to check if that row is tall enough for the image.
			// We want to fit on a row as far left as possible.
			if((row->bounds[1].x < leftest && row->bounds[1].y - row->bounds[0].y >= lt->sizeY * tfi->tiletimes.y) || addto == NULL)
			{
				addto = &*row;
				leftest = row->bounds[1].x;
			}
		}

		// If adding it on a row as far left as possible is still making the image more than twice as wide as it is tall,
		// add a new row, and put the image on that row.

		if(leftest + lt->sizeX > maxdim.y * 2)
		{
			TexFitRow* lastrow = &*rows.rbegin();
			TexFitRow row;

			row.bounds[0].x = 0;
			row.bounds[0].y = lastrow->bounds[1].y;
			row.bounds[1].x = lt->sizeX * tfi->tiletimes.x;
			row.bounds[1].y = row.bounds[0].y + lt->sizeY * tfi->tiletimes.y;
			maxdim.y = row.bounds[1].y;

			TexFit fit;
			fit.bounds[0].x = row.bounds[0].x;
			fit.bounds[0].y = row.bounds[0].y;
			fit.bounds[1].x = row.bounds[1].x;
			fit.bounds[1].y = row.bounds[1].y;
			fit.texindex = *hs;
			memcpy(tfi->bounds, fit.bounds, sizeof(Vec2i)*2);

			if(fit.bounds[1].x > maxdim.x)
				maxdim.x = fit.bounds[1].x;

			row.fits.push_back(fit);
			rows.push_back(row);

#ifdef COMPILEB_DEBUG
			g_log<<"\tfit2 to "<<fit.bounds[0].x<<","<<fit.bounds[0].y<<"->"<<fit.bounds[1].x<<","<<fit.bounds[1].y<<endl;
			g_log.flush();
#endif

			continue;
		}

		// Added it on to the right of the row.

		TexFit fit;
		fit.bounds[0].x = addto->bounds[1].x;
		fit.bounds[0].y = addto->bounds[0].y;
		fit.bounds[1].x = fit.bounds[0].x + lt->sizeX * tfi->tiletimes.x;
		fit.bounds[1].y = addto->bounds[0].y + lt->sizeY * tfi->tiletimes.y;
		fit.texindex = *hs;
		memcpy(tfi->bounds, fit.bounds, sizeof(Vec2i)*2);

		addto->fits.push_back(fit);
		addto->bounds[1].x += lt->sizeX * tfi->tiletimes.x;

		if(addto->bounds[1].x > maxdim.x)
			maxdim.x = addto->bounds[1].x;

#ifdef COMPILEB_DEBUG
		g_log<<"\tfit3 to "<<fit.bounds[0].x<<","<<fit.bounds[0].y<<"->"<<fit.bounds[1].x<<","<<fit.bounds[1].y<<endl;
		g_log.flush();
#endif
	}
}

static Vec2i finaldim;
static float scalex;
static float scaley;
static LoadedTex* resizedimages;

void ResizeImages()
{
	finaldim = Vec2i( Max2Pow( maxdim.x ), Max2Pow( maxdim.y ) );
	scalex = (float)finaldim.x / (float)maxdim.x;
	scaley = (float)finaldim.y / (float)maxdim.y;

#ifdef COMPILEB_DEBUG
	g_log<<"dimensions = ("<<maxdim.x<<","<<maxdim.y<<")->("<<finaldim.x<<","<<finaldim.y<<")"<<endl;
	g_log.flush();
#endif

	resizedimages = new LoadedTex[ uniquetexs.size()*3 ];

	for(int i=0; i<uniquetexs.size()*3; i++)
		resizedimages[i].data = NULL;

#ifdef COMPILEB_DEBUG
	int counter = 0;
#endif

	for(auto r=rows.begin(); r!=rows.end(); r++)
	{
		for(auto f=r->fits.begin(); f!=r->fits.end(); f++)
		{
			TexRef* tr = &texref[ f->texindex ];
			TexFitInfo* tfi = &texfitinfo[ f->texindex ];

			tfi->newdim.x = (f->bounds[1].x - f->bounds[0].x) / (float)tfi->tiletimes.x * scalex;
			tfi->newdim.y = (f->bounds[1].y - f->bounds[0].y) / (float)tfi->tiletimes.y * scaley;

#ifdef COMPILEB_DEBUG
			g_log<<"compile 6 image ("<<(f->bounds[1].x - f->bounds[0].x)<<","<<(f->bounds[1].y - f->bounds[0].y)<<")->("<<tfi->newdim.x<<","<<tfi->newdim.y<<")"<<counter<<endl;
			g_log.flush();
			counter ++;
#endif

			Vec2i newdim( ceil(tfi->newdim.x), ceil(tfi->newdim.y) );

			Resample(images[ tr->diffindex ], &resizedimages[ tr->diffindex ], newdim);

#ifdef COMPILEB_DEBUG
			g_log<<"\t done compile 6 image diff"<<endl;
			g_log.flush();
#endif

			Resample(images[ tr->specindex ], &resizedimages[ tr->specindex ], newdim);

#ifdef COMPILEB_DEBUG
			g_log<<"\t done compile 6 image spec"<<endl;
			g_log.flush();
#endif

			Resample(images[ tr->normindex ], &resizedimages[ tr->normindex ], newdim);

#ifdef COMPILEB_DEBUG
			g_log<<"\t done compile 6 image norm"<<endl;
			g_log.flush();
#endif
		}
	}
}

static LoadedTex finaldiff;
static LoadedTex finalspec;
static LoadedTex finalnorm;

void CombineImages()
{
#ifdef COMPILEB_DEBUG
	g_log<<"compile 7a"<<endl;
	g_log.flush();
#endif

	AllocTex(&finaldiff, finaldim.x, finaldim.y, transparency ? 4 : 3);
	AllocTex(&finalspec, finaldim.x, finaldim.y, 3);
	AllocTex(&finalnorm, finaldim.x, finaldim.y, 3);

#ifdef COMPILEB_DEBUG
	g_log<<"compile 7b"<<endl;
	g_log.flush();
#endif

	for(auto r=rows.begin(); r!=rows.end(); r++)
	{
		for(auto f=r->fits.begin(); f!=r->fits.end(); f++)
		{
			TexRef* tr = &texref[ f->texindex ];
			TexFitInfo* tfi = &texfitinfo[ f->texindex ];

			tfi->bounds[0] = f->bounds[0];
			tfi->bounds[1] = f->bounds[2];

#ifdef COMPILEB_DEBUG
			g_log<<"compile 7b1"<<endl;
			g_log.flush();
#endif

			for(int x=0; x<tfi->tiletimes.x; x++)
				for(int y=0; y<tfi->tiletimes.y; y++)
				{
					Vec2i newpos;
					newpos.x = f->bounds[0].x * scalex + x * tfi->newdim.x;
					newpos.y = f->bounds[0].y * scaley + y * tfi->newdim.y;

#ifdef COMPILEB_DEBUG
					g_log<<"compile 7b2 to "<<newpos.x<<","<<newpos.y<<endl;
					g_log.flush();
#endif

					Blit( &resizedimages[ tr->diffindex ], &finaldiff, newpos );
					Blit( &resizedimages[ tr->specindex ], &finalspec, newpos );
					Blit( &resizedimages[ tr->normindex ], &finalnorm, newpos );
				}
#ifdef COMPILEB_DEBUG
				g_log<<"compile 7b3"<<endl;
				g_log.flush();
#endif
		}
	}

#ifdef COMPILEB_DEBUG
	g_log<<"compile 7c"<<endl;
	g_log.flush();
#endif

	for(int i=0; i<uniquetexs.size()*3; i++)
	{

#ifdef COMPILEB_DEBUG
		g_log<<"compile 7c free image "<<i<<endl;
		g_log.flush();
#endif

		if(resizedimages[i].data)
			free(resizedimages[i].data);
	}

#ifdef COMPILEB_DEBUG
	g_log<<"compile 7d"<<endl;
	g_log.flush();
#endif

	delete [] resizedimages;

#ifdef COMPILEB_DEBUG
	g_log<<"compile 8"<<endl;
	g_log.flush();
#endif
}

static string difffull;
static string difffullpng;
static string specfull;
static string normfull;

void WriteImages()
{
	if(transparency)
		SavePNG(difffullpng.c_str(), &finaldiff);
	else
		SaveJPEG(difffull.c_str(), &finaldiff, 0.75f);
	SaveJPEG(specfull.c_str(), &finalspec, 0.75f);
	SaveJPEG(normfull.c_str(), &finalnorm, 0.75f);

	//SavePNG(difffullpng.c_str(), &finaldiff);
	//SavePNG(specfull.c_str(), &finalspec);
	//SavePNG(normfull.c_str(), &finalnorm);
}

void CalcTexCoords(EdMap* map, list<ModelHolder> &modelholders)
{
	int vindex = 0;

#ifdef COMPILEB_DEBUG
	unsigned int window3index;
	CreateTextureI(window3index, "textures/apartment/stuccowindow3.jpg", false);
#endif

	for(auto br=map->m_brush.begin(); br!=map->m_brush.end(); br++)
	{
		for(int i=0; i<br->m_nsides; i++)
		{
			BrushSide* s = &br->m_sides[i];

			if(s->m_diffusem == notexindex)
				continue;

			TexRef* tr = &texref[ s->m_diffusem ];
			LoadedTex* lt = images[ tr->diffindex ];
			TexFitInfo* tfi = &texfitinfo[ s->m_diffusem ];
			VertexArray* va = &s->m_drawva;

			for(int j=0; j<va->numverts; j+=3, vindex+=3)
			{
				//fullva.vertices[vindex] = va->vertices[j];
				//fullva.normals[vindex] = va->normals[j];

				Vec2f tc[3];
				tc[0] = va->texcoords[j + 0];
				tc[1] = va->texcoords[j + 1];
				tc[2] = va->texcoords[j + 2];

				float minu = min(tc[0].x, min(tc[1].x, tc[2].x));
				float minv = min(tc[0].y, min(tc[1].y, tc[2].y));
				float maxu = max(tc[0].x, max(tc[1].x, tc[2].x));
				float maxv = max(tc[0].y, max(tc[1].y, tc[2].y));

				float rangeu = maxu - minu;
				float rangev = maxv - minv;

				float offu = minu - floor(minu);
				float offv = minv - floor(minv);

				for(int k=0; k<3; k++)
				{
					float uvpixel[2];

					uvpixel[0] = tc[k].x * (float)lt->sizeX;
					uvpixel[1] = tc[k].y * (float)lt->sizeY;

					uvpixel[0] = uvpixel[0] - minu*(float)lt->sizeX + offu*(float)lt->sizeX;
					uvpixel[1] = uvpixel[1] - minv*(float)lt->sizeY + offv*(float)lt->sizeY;

					uvpixel[0] += tfi->bounds[0].x;
					uvpixel[1] += tfi->bounds[0].y;

					fullva.texcoords[vindex + k].x = uvpixel[0] / (float)maxdim.x;
					fullva.texcoords[vindex + k].y = uvpixel[1] / (float)maxdim.y;
				}
			}
		}
	}

	for(auto mhiter = modelholders.begin(); mhiter != modelholders.end(); mhiter++)
	{
		VertexArray* va = &mhiter->frames[0];
		Model* pmodel = &g_model[mhiter->model];
		TexRef* tr = &texref[ pmodel->m_diffusem ];
		LoadedTex* lt = images[ tr->diffindex ];
		TexFitInfo* tfi = &texfitinfo[ pmodel->m_diffusem ];

		for(int j=0; j<va->numverts; j+=3, vindex+=3)
		{
			Vec2f tc[3];
			tc[0] = va->texcoords[j + 0];
			tc[1] = va->texcoords[j + 1];
			tc[2] = va->texcoords[j + 2];

			float minu = min(tc[0].x, min(tc[1].x, tc[2].x));
			float minv = min(tc[0].y, min(tc[1].y, tc[2].y));
			float maxu = max(tc[0].x, max(tc[1].x, tc[2].x));
			float maxv = max(tc[0].y, max(tc[1].y, tc[2].y));

			float rangeu = maxu - minu;
			float rangev = maxv - minv;

			float offu = minu - floor(minu);
			float offv = minv - floor(minv);

			for(int k=0; k<3; k++)
			{
				float uvpixel[2];

				uvpixel[0] = tc[k].x * (float)lt->sizeX;
				uvpixel[1] = tc[k].y * (float)lt->sizeY;

				uvpixel[0] = uvpixel[0] - minu*(float)lt->sizeX + offu*(float)lt->sizeX;
				uvpixel[1] = uvpixel[1] - minv*(float)lt->sizeY + offv*(float)lt->sizeY;

				uvpixel[0] += tfi->bounds[0].x;
				uvpixel[1] += tfi->bounds[0].y;

				fullva.texcoords[vindex + k].x = uvpixel[0] / (float)maxdim.x;
				fullva.texcoords[vindex + k].y = uvpixel[1] / (float)maxdim.y;
			}
		}
	}
}

void SaveModel(const char* fullfile)
{
	FILE* fp = fopen(fullfile, "wb");

	char tag[] = TAG_BUILDINGM;
	fwrite(tag, sizeof(char), 5, fp);

	float version = BUILDINGM_VERSION;
	fwrite(&version, sizeof(float), 1, fp);

	SaveVertexArray(fp, &fullva);

	fclose(fp);
}

void CleanupModelCompile()
{
	if(finaldiff.data)
		free(finaldiff.data);
	if(finalspec.data)
		free(finalspec.data);
	if(finalnorm.data)
		free(finalnorm.data);

	for(int i=0; i<uniquetexs.size()*3; i++)
	{
		if(images[i])
		{
			if(images[i]->data)
			{
				free(images[i]->data);
			}

			free(images[i]);
		}
	}

	delete [] images;

	fullva.free();
}

void CompileModel(const char* fullfile, EdMap* map, list<ModelHolder> &modelholders)
{
	char basename[MAX_PATH+1];
	strcpy(basename, fullfile);
	StripPath(basename);
	StripExtension(basename);

	string fullpath = StripFile(fullfile);

	difffull = fullpath + string(basename) + ".jpg";
	difffullpng = fullpath + string(basename) + ".png";
	specfull = fullpath + string(basename) + ".spec.jpg";
	normfull = fullpath + string(basename) + ".norm.jpg";

	//string diffpath = string(basename) + ".jpg";
	//string diffpathpng = string(basename) + ".png";
	//string specpath = string(basename) + ".spec.jpg";
	//string normpath = string(basename) + ".norm.jpg";
	
	CreateTexture(notexindex, "textures/notex.jpg", false);

	transparency = false;

	// STEP 1:
	// i. count total triangles, excluding sides with default (notex.jpg) texture (they will not be added to the final building)
	// ii. make a list of unique textures used in the brush sides and see if there are any transparent ones
	// iii. calculate the max x,y tile extents of the texture coordinates for each triangles

	ntris = 0;
	uniquetexs.clear();

	CountTrisModel(modelholders);
	ListUniqueTexsModel(modelholders);
	CountTrisBrush(map);
	ListUniqueTexsBrush(map);

	// STEP 2.
	// i. allocate the final array of all the vertices/texcoords/normals that will be written to file.
	// ii. store the vertices for now and normals for now. texcoords will be calculated later when the texture images are packed together.

	AllocFinalVerts(map, modelholders);

	// STEP 3.
	// i. load all the diffuse, specular, and normal texture images (RGB data) from the compiled list of unique texture indices
	// ii. save references to the texture images (RGB data) array based on the unique (diffuse) texture index

	LoadAllRGBData();

	// STEP 4.
	// i. make a list of images sorted by height from tallest to shortest

	HeightSort();

	// STEP 5.
	// i. add textures to the right side of the row until the width of the final image is twice its height
	// ii. then increase the height and add a new row
	// iii. for each next image, check each row if the image can be added without increasing the image width
	// iii. repeat until all diffuse images have been fitted in
	// iv. calculate the scaled, translated texture coordinates for the final vertex array

	FitImages();

	// STEP 6.
	// i. resize images so that the combined final image is a power of 2 that doesn't exceed 2048x2048

	ResizeImages();

	// STEP 7.
	// i. combine all the resized images into the final image
	CombineImages();

	// STEP 8.
	// i. write the JPEG's/PNG's

	WriteImages();

	// STEP 9.
	// i. calculate the new texcoords
	// ii. write the vertex array

	CalcTexCoords(map, modelholders);
	SaveModel(fullfile);

	// Free the diffuse, specular, normal map RGB data

	CleanupModelCompile();
}