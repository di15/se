#include "gui.h"
#include "../texture.h"

void ViewLayer::draw()
{
	if(!m_opened)
		return;

	for(auto w=m_subwidg.begin(); w!=m_subwidg.end(); w++)
		(*w)->draw();

	subdraw();
}

void ViewLayer::drawover()
{
	if(!m_opened)
		return;

	for(auto w=m_subwidg.begin(); w!=m_subwidg.end(); w++)
		(*w)->drawover();

	subdrawover();
}

void ViewLayer::inev(InEv* ie)
{
	if(!m_opened)
		return;

#if 0
	for(auto w=m_subwidg.rbegin(); w!=m_subwidg.rend(); w++)
		(*w)->inev(ie);
#else
	//safe, may shift during call
	int win = 0;
	while(win < m_subwidg.size())
	{
		int win2 = 0;
		for(auto wit=m_subwidg.rbegin(); wit!=m_subwidg.rend(); wit++, win2++)
		{
			if(win2 < win)
				continue;

			(*wit)->inev(ie);
			break;
		}
		win++;
	}
#endif

	subinev(ie);
}

void ViewLayer::reframe()
{
	m_pos[0] = 0;
	m_pos[1] = 0;
	m_pos[2] = g_width-(float)1;
	m_pos[3] = g_height-(float)1;

	if(reframefunc)
		reframefunc(this);

	for(auto w=m_subwidg.begin(); w!=m_subwidg.end(); w++)
		(*w)->reframe();

	subreframe();
}
