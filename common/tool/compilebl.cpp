

#include "compilebl.h"
#include "../save/edmap.h"
#include "../utils.h"
#include "../platform.h"
#include "../texture.h"
#include "../save/save.h"
#include "../save/saveedm.h"


//#define COMPILEB_DEBUG

#define TEXTYPES		4

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

	while( twopow < lowerbound
#if 0
		&& twopow < 2048
#endif
		)
		twopow *= 2;

	return twopow;
}

int Max2Pow32(int lowerbound)
{
	int twopow = 32;

	while( twopow < lowerbound
#if 0
		&& twopow < 2048
#endif
		)
		twopow *= 2;

	return twopow;
}

// determine how many times a triangle tiles on a texture
Vec2i TileTimes(Vec2f* tc)
{
	Vec2f tc0 = tc[0];
	Vec2f tc1 = tc[1];
	Vec2f tc2 = tc[2];

	float minu = fmin(tc0.x, fmin(tc1.x, tc2.x));
	float minv = fmin(tc0.y, fmin(tc1.y, tc2.y));
	float maxu = fmax(tc0.x, fmax(tc1.x, tc2.x));
	float maxv = fmax(tc0.y, fmax(tc1.y, tc2.y));

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
		g_applog<<"--------------11 or 11 tile times---------------"<<std::endl;
		g_applog<<"ceilrange = "<<ceilrangeu<<","<<ceilrangev<<std::endl;
		g_applog<<"range = "<<rangeu<<","<<rangev<<std::endl;
		g_applog<<"min = "<<minu<<","<<minv<<std::endl;
		g_applog<<"max = "<<maxu<<","<<maxv<<std::endl;
		g_applog<<"[0] = "<<tc0.x<<","<<tc0.y<<std::endl;
		g_applog<<"[1] = "<<tc1.x<<","<<tc1.y<<std::endl;
		g_applog<<"[2] = "<<tc2.x<<","<<tc2.y<<std::endl;
		g_applog<<"--------------11 or 11 tile times---------------"<<std::endl;
		g_applog.flush();
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
	g_applog<<"resample...?"<<std::endl;
	g_applog.flush();
#endif

	if(original == NULL || original->data == NULL || original->sizeX <= 0 || original->sizeY <= 0)
	{
#ifdef COMPILEB_DEBUG
		g_applog<<"resample NULL 1"<<std::endl;
		g_applog.flush();
#endif

		empty->data = NULL;

#ifdef COMPILEB_DEBUG
		g_applog<<"resample NULL 2"<<std::endl;
		g_applog.flush();
#endif

		empty->sizeX = 0;
		empty->sizeY = 0;

		if(original != NULL)
			empty->channels = original->channels;

		return;
	}

#ifdef COMPILEB_DEBUG
	g_applog<<"resample "<<original->sizeX<<","<<original->sizeY<<" to "<<newdim.x<<","<<newdim.y<<std::endl;
	g_applog.flush();
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
	g_applog<<"\t done resample"<<std::endl;
	g_applog.flush();
#endif
}

static bool transparency = false;
static unsigned int notexindex;
static int ntris = 0;
static std::list<unsigned int> uniquetexs;
static TexFitInfo texfitinfo[TEXTURES];

#if 1	//commented, until I update the code to use ModelPart and etc.

void CountTrisModel(std::list<ModelHolder> &modelholders)
{
	for(auto mhiter = modelholders.begin(); mhiter != modelholders.end(); mhiter++)
	{
		Model* pmodel = &g_model[mhiter->model];

		VertexArray* pva = &pmodel->m_va[0];

		ntris += pva->numverts / 3;
	}
}

void ListUniqueTexsModel(std::list<ModelHolder> &modelholders)
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

void AllocFinalVerts(EdMap* map, std::list<ModelHolder> &modelholders)
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
	images = new LoadedTex*[uniquetexs.size()*TEXTYPES];

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
		g_applog<<"tile times "<<texref[ *ut ].filepath<<" "<<tfi->tiletimes.x<<","<<tfi->tiletimes.y<<std::endl;
		g_applog.flush();
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

		i++;

		char fullownerpath[MAX_PATH+1];
		OwnPath(basepath, fullownerpath);
		images[i] = LoadTexture(fullownerpath);
		tr->ownindex = i;

#ifdef COMPILEB_DEBUG
		g_applog<<"------"<<std::endl;
		if(images[tr->diffindex])
			g_applog<<fulldiffusepath<<" ("<<images[tr->diffindex]->sizeX<<","<<images[tr->diffindex]->sizeY<<")"<<std::endl;
		if(images[tr->specindex])
			g_applog<<fullspecularpath<<" ("<<images[tr->specindex]->sizeX<<","<<images[tr->specindex]->sizeY<<")"<<std::endl;
		if(images[tr->normindex])
			g_applog<<fullnormalpath<<" ("<<images[tr->normindex]->sizeX<<","<<images[tr->normindex]->sizeY<<")"<<std::endl;
		g_applog<<i<<std::endl;
		g_applog<<"------"<<std::endl;
		g_applog.flush();
#endif
	}
}

static std::list<unsigned int> heightsorted;

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
				g_applog<<thisheight<<" >= "<<lastheight<<std::endl;
				g_applog.flush();
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
static std::list<TexFitRow> rows;

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
		g_applog<<"------height sorted-----"<<std::endl;
		g_applog<<"\t "<<tr->filepath<<std::endl;
		g_applog<<"\t ("<<lt->sizeX<<","<<lt->sizeY<<")"<<std::endl;
		g_applog<<"------height sorted-----"<<std::endl;
		g_applog.flush();
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
			g_applog<<"\tfit1 to "<<fit.bounds[0].x<<","<<fit.bounds[0].y<<"->"<<fit.bounds[1].x<<","<<fit.bounds[1].y<<std::endl;
			g_applog.flush();
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
			g_applog<<"\tfit2 to "<<fit.bounds[0].x<<","<<fit.bounds[0].y<<"->"<<fit.bounds[1].x<<","<<fit.bounds[1].y<<std::endl;
			g_applog.flush();
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
		g_applog<<"\tfit3 to "<<fit.bounds[0].x<<","<<fit.bounds[0].y<<"->"<<fit.bounds[1].x<<","<<fit.bounds[1].y<<std::endl;
		g_applog.flush();
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
	g_applog<<"dimensions = ("<<maxdim.x<<","<<maxdim.y<<")->("<<finaldim.x<<","<<finaldim.y<<")"<<std::endl;
	g_applog.flush();
#endif

	resizedimages = new LoadedTex[ uniquetexs.size()*TEXTYPES ];

	for(int i=0; i<uniquetexs.size()*TEXTYPES; i++)
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
			g_applog<<"compile 6 image ("<<(f->bounds[1].x - f->bounds[0].x)<<","<<(f->bounds[1].y - f->bounds[0].y)<<")->("<<tfi->newdim.x<<","<<tfi->newdim.y<<")"<<counter<<std::endl;
			g_applog.flush();
			counter ++;
#endif

			Vec2i newdim( ceil(tfi->newdim.x), ceil(tfi->newdim.y) );

			Resample(images[ tr->diffindex ], &resizedimages[ tr->diffindex ], newdim);

#ifdef COMPILEB_DEBUG
			g_applog<<"\t done compile 6 image diff"<<std::endl;
			g_applog.flush();
#endif

			Resample(images[ tr->specindex ], &resizedimages[ tr->specindex ], newdim);

#ifdef COMPILEB_DEBUG
			g_applog<<"\t done compile 6 image spec"<<std::endl;
			g_applog.flush();
#endif

			Resample(images[ tr->normindex ], &resizedimages[ tr->normindex ], newdim);

#ifdef COMPILEB_DEBUG
			g_applog<<"\t done compile 6 image norm"<<std::endl;
			g_applog.flush();
#endif

			Resample(images[ tr->ownindex ], &resizedimages[ tr->ownindex ], newdim);
		}
	}
}

static LoadedTex finaldiff;
static LoadedTex finalspec;
static LoadedTex finalnorm;
static LoadedTex finalown;

void CombineImages()
{
#ifdef COMPILEB_DEBUG
	g_applog<<"compile 7a"<<std::endl;
	g_applog.flush();
#endif

	AllocTex(&finaldiff, finaldim.x, finaldim.y, transparency ? 4 : 3);
	AllocTex(&finalspec, finaldim.x, finaldim.y, 3);
	AllocTex(&finalnorm, finaldim.x, finaldim.y, 3);
	AllocTex(&finalown, finaldim.x, finaldim.y, 4);

#ifdef COMPILEB_DEBUG
	g_applog<<"compile 7b"<<std::endl;
	g_applog.flush();
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
			g_applog<<"compile 7b1"<<std::endl;
			g_applog.flush();
#endif

			for(int x=0; x<tfi->tiletimes.x; x++)
				for(int y=0; y<tfi->tiletimes.y; y++)
				{
					Vec2i newpos;
					newpos.x = f->bounds[0].x * scalex + x * tfi->newdim.x;
					newpos.y = f->bounds[0].y * scaley + y * tfi->newdim.y;

#ifdef COMPILEB_DEBUG
					g_applog<<"compile 7b2 to "<<newpos.x<<","<<newpos.y<<std::endl;
					g_applog.flush();
#endif

					Blit( &resizedimages[ tr->diffindex ], &finaldiff, newpos );
					Blit( &resizedimages[ tr->specindex ], &finalspec, newpos );
					Blit( &resizedimages[ tr->normindex ], &finalnorm, newpos );
				}
#ifdef COMPILEB_DEBUG
				g_applog<<"compile 7b3"<<std::endl;
				g_applog.flush();
#endif
		}
	}

#ifdef COMPILEB_DEBUG
	g_applog<<"compile 7c"<<std::endl;
	g_applog.flush();
#endif

#if 1
	for(int i=0; i<uniquetexs.size()*TEXTYPES; i++)
	{

#ifdef COMPILEB_DEBUG
		g_applog<<"compile 7c free image "<<i<<std::endl;
		g_applog.flush();
#endif

#if 0
		if(resizedimages[i].data)
			free(resizedimages[i].data);
#endif
		resizedimages[i].destroy();
	}
#endif

#ifdef COMPILEB_DEBUG
	g_applog<<"compile 7d"<<std::endl;
	g_applog.flush();
#endif

	delete [] resizedimages;

#ifdef COMPILEB_DEBUG
	g_applog<<"compile 8"<<std::endl;
	g_applog.flush();
#endif
}

static std::string difffull;
static std::string difffullpng;
static std::string specfull;
static std::string normfull;
static std::string ownfull;

void WriteImages()
{
	if(transparency)
		SavePNG(difffullpng.c_str(), &finaldiff);
	else
		SaveJPEG(difffull.c_str(), &finaldiff, 0.75f);
	SaveJPEG(specfull.c_str(), &finalspec, 0.75f);
	SaveJPEG(normfull.c_str(), &finalnorm, 0.75f);
	SavePNG(ownfull.c_str(), &finalown);

	//SavePNG(difffullpng.c_str(), &finaldiff);
	//SavePNG(specfull.c_str(), &finalspec);
	//SavePNG(normfull.c_str(), &finalnorm);
}

void CalcTexCoords(EdMap* map, std::list<ModelHolder> &modelholders)
{
	int vindex = 0;

#ifdef COMPILEB_DEBUG
	//unsigned int window3index;
	//CreateTextureI(window3index, "textures/apartment/stuccowindow3.jpg", false);
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

				float minu = fmin(tc[0].x, fmin(tc[1].x, tc[2].x));
				float minv = fmin(tc[0].y, fmin(tc[1].y, tc[2].y));
				float maxu = fmax(tc[0].x, fmax(tc[1].x, tc[2].x));
				float maxv = fmax(tc[0].y, fmax(tc[1].y, tc[2].y));

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

			float minu = fmin(tc[0].x, fmin(tc[1].x, tc[2].x));
			float minv = fmin(tc[0].y, fmin(tc[1].y, tc[2].y));
			float maxu = fmax(tc[0].x, fmax(tc[1].x, tc[2].x));
			float maxv = fmax(tc[0].y, fmax(tc[1].y, tc[2].y));

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
#if 0
	if(finaldiff.data)
		free(finaldiff.data);
	if(finalspec.data)
		free(finalspec.data);
	if(finalnorm.data)
		free(finalnorm.data);
#endif

	finaldiff.destroy();
	finalspec.destroy();
	finalnorm.destroy();
	finalown.destroy();

	for(int i=0; i<uniquetexs.size()*TEXTYPES; i++)
	{
		if(images[i])
		{
#if 0
			if(images[i]->data)
			{
				free(images[i]->data);
			}
#endif
			images[i]->destroy();
			//free(images[i]);
			delete images[i];
		}
	}

	delete [] images;

	fullva.free();
}

void CompileModel(const char* fullfile, EdMap* map, std::list<ModelHolder> &modelholders)
{
	char basename[MAX_PATH+1];
	strcpy(basename, fullfile);
	StripPath(basename);
	StripExtension(basename);

	std::string fullpath = StripFile(fullfile);

	difffull = fullpath + std::string(basename) + ".jpg";
	difffullpng = fullpath + std::string(basename) + ".png";
	specfull = fullpath + std::string(basename) + ".spec.jpg";
	normfull = fullpath + std::string(basename) + ".norm.jpg";
	ownfull = fullpath + std::string(basename) + ".team.png";

	//std::string diffpath = std::string(basename) + ".jpg";
	//std::string diffpathpng = std::string(basename) + ".png";
	//std::string specpath = std::string(basename) + ".spec.jpg";
	//std::string normpath = std::string(basename) + ".norm.jpg";

	CreateTexture(notexindex, "textures/notex.jpg", false, true);

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

#endif