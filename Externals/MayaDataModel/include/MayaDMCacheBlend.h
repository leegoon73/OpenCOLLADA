/*
    Copyright (c) 2008 NetAllied Systems GmbH

    This file is part of MayaDataModel.

    Licensed under the MIT Open Source License,
    for details please see LICENSE file or the website
    http://www.opensource.org/licenses/mit-license.php
*/
#ifndef __MayaDM_CACHEBLEND_H__
#define __MayaDM_CACHEBLEND_H__
#include "MayaDMTypes.h"
#include "MayaDMConnectables.h"
#include "MayaDMCacheBase.h"
namespace MayaDM
{
class CacheBlend : public CacheBase
{
public:
	struct InCache{
		doubleArray* perPtWeights;
		void write(FILE* file) const
		{
			size_t size = sizeof(perPtWeights)/sizeof(doubleArray);
			for(size_t i=0; i<size; ++i)
			{
				perPtWeights[i].write(file);
				if(i+1<size) fprintf(file," ");
			}
		}
	};
	struct CacheData{
		bool range;
		double weight;
		void write(FILE* file) const
		{
			fprintf(file,"%i ", range);
			fprintf(file,"%f", weight);
		}
	};
public:
	CacheBlend(FILE* file,const std::string& name,const std::string& parent=""):CacheBase(file, name, parent, "cacheBlend"){}
	virtual ~CacheBlend(){}
	void setInCache(size_t ic_i,const InCache& ic)
	{
		fprintf(mFile,"setAttr \".ic[%i]\" ",ic_i);
		ic.write(mFile);
		fprintf(mFile,";\n");

	}
	void setPerPtWeights(size_t ic_i,size_t ppw_i,const doubleArray& ppw)
	{
		if(ppw.size == 0) return;
		fprintf(mFile,"setAttr \".ic[%i].ppw[%i]\" -type \"doubleArray\" ",ic_i,ppw_i);
		ppw.write(mFile);
		fprintf(mFile,";\n");

	}
	void setPerPtWeights(size_t ic_i,size_t ppw_start,size_t ppw_end,doubleArray* ppw)
	{
		fprintf(mFile,"setAttr \".ic[%i].ppw[%i:%i]\" ", ic_i,ppw_start,ppw_end);
		size_t size = (ppw_end-ppw_start)*1+1;
		for(size_t i=0;i<size;++i)
		{
			ppw[i].write(mFile);
			fprintf(mFile,"\n");
		}
		fprintf(mFile,";\n");

	}
	void startPerPtWeights(size_t ic_i,size_t ppw_start,size_t ppw_end)
	{
		fprintf(mFile,"setAttr \".ic[%i].ppw[%i:%i]\"",ic_i,ppw_start,ppw_end);
		fprintf(mFile," -type \"doubleArray\" ");

	}
	void appendPerPtWeights(const doubleArray& ppw)
	{
		fprintf(mFile,"\n");
		ppw.write(mFile);

	}
	void endPerPtWeights()
	{
		fprintf(mFile,";\n");

	}
	void setDisableAll(bool da)
	{
		if(da == 0) return;
		fprintf(mFile,"setAttr \".da\" %i;\n", da);

	}
	void setCacheData(size_t cd_i,const CacheData& cd)
	{
		fprintf(mFile,"setAttr \".cd[%i]\" ",cd_i);
		cd.write(mFile);
		fprintf(mFile,";\n");

	}
	void setRange(size_t cd_i,bool ra)
	{
		if(ra == false) return;
		fprintf(mFile,"setAttr \".cd[%i].ra\" %i;\n", cd_i,ra);

	}
	void setWeight(size_t cd_i,double w)
	{
		if(w == 1.0) return;
		fprintf(mFile,"setAttr \".cd[%i].w\" %f;\n", cd_i,w);

	}
	void getInCache(size_t ic_i)
	{
		fprintf(mFile,"\"%s.ic[%i]\"",mName.c_str(),ic_i);

	}
	void getVectorArray(size_t ic_i,size_t va_i)
	{
		fprintf(mFile,"\"%s.ic[%i].va[%i]\"",mName.c_str(),ic_i,va_i);

	}
	void getPerPtWeights(size_t ic_i,size_t ppw_i)
	{
		fprintf(mFile,"\"%s.ic[%i].ppw[%i]\"",mName.c_str(),ic_i,ppw_i);

	}
	void getDisableAll()
	{
		fprintf(mFile,"\"%s.da\"",mName.c_str());

	}
	void getCacheData(size_t cd_i)
	{
		fprintf(mFile,"\"%s.cd[%i]\"",mName.c_str(),cd_i);

	}
	void getStart(size_t cd_i)
	{
		fprintf(mFile,"\"%s.cd[%i].st\"",mName.c_str(),cd_i);

	}
	void getEnd(size_t cd_i)
	{
		fprintf(mFile,"\"%s.cd[%i].e\"",mName.c_str(),cd_i);

	}
	void getRange(size_t cd_i)
	{
		fprintf(mFile,"\"%s.cd[%i].ra\"",mName.c_str(),cd_i);

	}
	void getWeight(size_t cd_i)
	{
		fprintf(mFile,"\"%s.cd[%i].w\"",mName.c_str(),cd_i);

	}
protected:
	CacheBlend(FILE* file,const std::string& name,const std::string& parent,const std::string& nodeType):CacheBase(file, name, parent, nodeType) {}

};
}//namespace MayaDM
#endif//__MayaDM_CACHEBLEND_H__