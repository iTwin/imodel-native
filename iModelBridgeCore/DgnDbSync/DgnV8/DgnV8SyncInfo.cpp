/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"

#define MUSTBEDBRESULT(stmt,RESULT) {auto rc = stmt; if (rc != RESULT) {SetLastError(rc); return BSIERROR;}}
#define MUSTBEOK(stmt) MUSTBEDBRESULT(stmt,BE_SQLITE_OK)
#define MUSTBEROW(stmt) MUSTBEDBRESULT(stmt,BE_SQLITE_ROW)
#define MUSTBEDONE(stmt) MUSTBEDBRESULT(stmt,BE_SQLITE_DONE)

#define DUMP_HASHES
#if defined (DUMP_HASHES)
static uint32_t s_lowSize = UINT32_MAX, s_highSize=0, s_count=0;
static bool s_dumpHash;
static FILE* s_hashDumpFile;
#endif

#if defined (DUMP_HASHES)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Dehorty                    09/92
+---------------+---------------+---------------+---------------+---------------+------*/
static void dumpRawBytes(Byte* rsc, size_t rscSize)
    {
    char    tmpstr[37];
    size_t  i, k=0;

#if defined (VERBOSE_XML_STYLE)
    fprintf(s_hashDumpFile, "<data>\n");
#endif

    memset(tmpstr, 0, 37);
    for (i=0; i<rscSize; i++)
        {
        k = i % 16;
#if defined (VERBOSE_XML_STYLE)
        if (k == 0)
            fprintf(s_hashDumpFile, "%.6x  ", i);
#endif
        fprintf(s_hashDumpFile, "%.2x ", rsc[i]);

        tmpstr[k] = isprint(rsc[i]) ? rsc[i] : '.';

        if (k == 15)
            {
            fprintf(s_hashDumpFile, "| %s\n", tmpstr);
            memset(tmpstr, 0, 37);
            }
        }

    if (k < 15)
        {
        for (i=0; i<(16-k-1); i++)
            fprintf(s_hashDumpFile, "   ");
        fprintf(s_hashDumpFile, "| %s\n", tmpstr);
        }

#if defined (VERBOSE_XML_STYLE)
    fprintf(s_hashDumpFile, "</data>\n");
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SamWilson       05/02
+---------------+---------------+---------------+---------------+---------------+------*/
static int     dumpLinkage(DgnV8Api::LinkageHeader const* pInLink, void const* data)
    {
    size_t lsz = 2*DgnV8Api::LinkageUtil::GetWords(pInLink);

#if defined (VERBOSE_XML_STYLE)
    fprintf(s_hashDumpFile, "<Linkage info=\"%d\", remote=\"%d\", modified=\"%d\", user=\"%d\", wdExponent=\"%d\", wdMantissa=\"%d\" (bytes=\"%d\"), primaryID=\"%d\">\n",
#else
    fprintf(s_hashDumpFile, "lnkge inf%d rem%d mod%d u%d exp%d man%d (%lldb) id%d\n",
#endif
        pInLink->info, pInLink->remote,
        pInLink->modified, pInLink->user,
        pInLink->wdExponent, pInLink->wdMantissa,
        (uint64_t)lsz, pInLink->primaryID);

    dumpRawBytes((Byte*)data, lsz-sizeof(DgnV8Api::LinkageHeader));


#if defined (VERBOSE_XML_STYLE)
    fprintf(s_hashDumpFile, "</Linkage>\n");
#endif
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SamWilson       05/02
+---------------+---------------+---------------+---------------+---------------+------*/
#if defined (VERBOSE_XML_STYLE)
#define DUMPEFIELD( n, d )      fprintf(s_hashDumpFile, "<ehdr_"          #n " value=\"%" #d "\"/>\n", el->ehdr.##n )
#define DUMPDFIELD( n, d )      fprintf(s_hashDumpFile, "<dhdr_"          #n " value=\"%" #d "\"/>\n", el->hdr.dhdr.##n )
#define DUMPPFIELD( n, d )      fprintf(s_hashDumpFile, "<dhdr_props_b."  #n " value=\"%" #d "\"/>\n", el->hdr.dhdr.props.b.##n )
#define DUMPSFIELD( n, d )      fprintf(s_hashDumpFile, "<dhdr_symb_"     #n " value=\"%" #d "\"/>\n", el->hdr.dhdr.symb.##n )
#define DUMPRFIELD( n, d )      fprintf(s_hashDumpFile, "<dhdr_range_"    #n " value=\"%" #d "\"/>\n", el->hdr.dhdr.range.##n )
#define DUMPELFIELD( e, n, d )  fprintf(s_hashDumpFile, "<"               #n " value=\"%" #d "\"/>\n", e->##n )
#else
#define DUMPEFIELD( n, d )      fprintf(s_hashDumpFile, #n "%" #d "\n", el->ehdr.##n )
#define DUMPDFIELD( n, d )      fprintf(s_hashDumpFile, #n "%" #d "\n", el->hdr.dhdr.##n )
#define DUMPPFIELD( n, d )      fprintf(s_hashDumpFile, #n "%" #d "\n", el->hdr.dhdr.props.b.##n )
#define DUMPSFIELD( n, d )      fprintf(s_hashDumpFile, #n "%" #d "\n", el->hdr.dhdr.symb.##n )
#define DUMPRFIELD( n, d )      fprintf(s_hashDumpFile, #n "%" #d "\n", el->hdr.dhdr.range.##n )
#define DUMPELFIELD( e, n, d )  fprintf(s_hashDumpFile, #n "%" #d "\n", e->##n )
#endif

static DgnV8Api::ModelId s_dumpModelId = UINT32_MAX;
static DgnV8Api::DgnFile* s_dumpFile = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SamWilson       05/02
+---------------+---------------+---------------+---------------+---------------+------*/
static void dumpDPoint3d(FILE* fp, DPoint3d const& pt)
    {
    fprintf(fp, "(%0.17lg,%0.17lg,%0.17lg)", pt.x, pt.y, pt.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SamWilson       05/02
+---------------+---------------+---------------+---------------+---------------+------*/
static void dumpDPoint2d(FILE* fp, DPoint2d const& pt)
    {
    fprintf(fp, "(%0.17lg,%0.17lg)", pt.x, pt.y);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SamWilson       05/02
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncInfo::DumpElement(DgnV8Api::ElementHandle const& veh)
    {
    if (!s_hashDumpFile)
        {
        BeAssert(false);
        return;
        }

    DgnV8Api::DgnModel* cache = veh.GetDgnModelP();

    #if defined (VERBOSE_XML_STYLE)
        fprintf(s_hashDumpFile, "\n<Element>\n");
    #else
        fprintf(s_hashDumpFile, "\n");
    #endif

    if (veh.GetDgnFileP() != s_dumpFile)
        {
        s_dumpFile = veh.GetDgnFileP();
        s_dumpModelId = UINT32_MAX;

        #if defined (VERBOSE_XML_STYLE)
            fprintf(s_hashDumpFile, "\n<DgnFile=\"%ls\" />\n", s_dumpFile->GetFileName().c_str());
        #else
            fprintf(s_hashDumpFile, "\nDgnFile=%ls\n\n", s_dumpFile->GetFileName().c_str());
        #endif // defined (VERBOSE_XML_STYLE)
        }

    if (cache)  // (NULL cache usualy means we are dumping the model header element for a completely empty model)
        {
        DgnV8Api::ModelId mid = cache->GetModelId();
        if (mid != s_dumpModelId)
            {
            #if defined (VERBOSE_XML_STYLE)
                fprintf(s_hashDumpFile, "\n<ModelId=\"%d\" />\n", mid);
            #else
                fprintf(s_hashDumpFile, "\nModel=%d\n\n", mid);
            #endif // defined (VERBOSE_XML_STYLE)
            s_dumpModelId = mid;
            }
        }

    DgnV8Api::MSElement const* el = veh.GetElementCP();
    Byte *pData = (Byte*)el;

    /*-----------------------------------------------------------------------------------
        Dump header
    -----------------------------------------------------------------------------------*/
    fprintf(s_hashDumpFile, "------------------%llu-------------\n", el->ehdr.uniqueId);
    //DUMPEFIELD(uniqueId,I64u);
    DUMPEFIELD(type,d);
    DUMPEFIELD(reserved,x);
    //DUMPEFIELD(archive,d);
    DUMPEFIELD(nonModel,d);
    DUMPEFIELD(locked,d);
    DUMPEFIELD(isGraphics,d);
    DUMPEFIELD(isComplexHeader,d);
    DUMPEFIELD(complex,d);
    DUMPEFIELD(deleted,d);
    DUMPEFIELD(elementSize,d);
    DUMPEFIELD(attrOffset,d);
    DUMPEFIELD(level,d);
    //DUMPEFIELD(lastModified,f);

    pData += sizeof (DgnV8Api::Elm_hdr);

    if (el->ehdr.isGraphics)
        {
        //DUMPDFIELD(grphgrp,d);
        DUMPDFIELD(priority,x);
        DUMPPFIELD(elementClass,d);
        DUMPPFIELD(invisible,d);
        DUMPPFIELD(unused2,d);
        //DUMPPFIELD(n,d);
        //DUMPPFIELD(m,d);
        DUMPPFIELD(is3d,d);
        DUMPPFIELD(r,d);
        DUMPPFIELD(p,d);
        DUMPPFIELD(s,d);
        DUMPPFIELD(h,d);
        DUMPDFIELD(reserved,d);
        DUMPSFIELD(style,d);
        DUMPSFIELD(weight,d);
        DUMPSFIELD(color,d);
        DUMPRFIELD(xlowlim,lld);
        DUMPRFIELD(ylowlim,lld);
        DUMPRFIELD(zlowlim,lld);
        DUMPRFIELD(xhighlim,lld);
        DUMPRFIELD(yhighlim,lld);
        DUMPRFIELD(yhighlim,lld);

        pData += sizeof (DgnV8Api::Disp_hdr);
        }

    /*-----------------------------------------------------------------------------------
        Dump element data that we understand with symbols
    -----------------------------------------------------------------------------------*/
    bool is3d = el->hdr.dhdr.props.b.is3d != 0;

    switch (el->ehdr.type)
        {
        case DgnV8Api::LINE_ELM:
            {
            if (is3d)
                {
                auto line = (DgnV8Api::Line_3d*)el;
                dumpDPoint3d(s_hashDumpFile, line->end);
                pData = (Byte*)(&line->end.z+1);
                }
            else
                {
                auto line = (DgnV8Api::Line_2d*)el;
                dumpDPoint2d(s_hashDumpFile, line->start);
                pData = (Byte*)(&line->end.y+1);
                }
            fprintf(s_hashDumpFile, "\n");
            break;
            }
        case DgnV8Api::LINE_STRING_ELM:
            {
            if (is3d)
                {
                auto line = (DgnV8Api::Line_String_3d*)el;
                for (uint32_t i=0; i<line->numverts; ++i)
                    dumpDPoint3d(s_hashDumpFile, line->vertice[i]);
                pData = (Byte*)(&line->vertice[line->numverts-1].z+1);
                }
            else
                {
                auto line = (DgnV8Api::Line_String_2d*)el;
                for (uint32_t i=0; i<line->numverts; ++i)
                    dumpDPoint2d(s_hashDumpFile, line->vertice[i]);
                pData = (Byte*)(&line->vertice[line->numverts-1].y+1);
                }
            fprintf(s_hashDumpFile, "\n");
            break;
            }

        case DgnV8Api::TEXT_NODE_ELM:
            {
            DgnV8Api::Text_node_3d *text_node_3d = (DgnV8Api::Text_node_3d*)el;
            DUMPELFIELD(text_node_3d,componentCount,d);
            DUMPELFIELD(text_node_3d,nodenumber,d);
            DUMPELFIELD(text_node_3d,font,d);
            DUMPELFIELD(text_node_3d,maxlngth,d);
            DUMPELFIELD(text_node_3d,just,d);
            DUMPELFIELD(text_node_3d,linespc,0.17lg);
            DUMPELFIELD(text_node_3d,lngthmult,0.17lg);
            DUMPELFIELD(text_node_3d,hghtmult,0.17lg);
            if (is3d)
                {
                DUMPELFIELD(text_node_3d,quat[0],0.17lg);
                DUMPELFIELD(text_node_3d,quat[1],0.17lg);
                DUMPELFIELD(text_node_3d,quat[2],0.17lg);
                DUMPELFIELD(text_node_3d,quat[3],0.17lg);
                DUMPELFIELD(text_node_3d,origin.x,0.17lg);
                DUMPELFIELD(text_node_3d,origin.y,0.17lg);
                DUMPELFIELD(text_node_3d,origin.z,0.17lg);
                pData = (Byte*)(&text_node_3d->origin.z+1);
                }
            else
                {
                DgnV8Api::Text_node_2d *text_node_2d = (DgnV8Api::Text_node_2d*)el;
                DUMPELFIELD(text_node_2d,rotationAngle,0.17lg);
                DUMPELFIELD(text_node_2d,origin.x,0.17lg);
                DUMPELFIELD(text_node_2d,origin.y,0.17lg);
                pData = (Byte*)(&text_node_2d->origin.y+1);
                }
            }
            break;

        case DgnV8Api::ATTRIBUTE_ELM:
            {
            DgnV8Api::AttributeElm *attribute = (DgnV8Api::AttributeElm*)el;
            /*DRange3d */DUMPELFIELD(attribute,range.org.x,0.17lg);     /* absolute range */
                            DUMPELFIELD(attribute,range.org.y,0.17lg);
                            DUMPELFIELD(attribute,range.org.z,0.17lg);
                            DUMPELFIELD(attribute,range.end.x,0.17lg);
                            DUMPELFIELD(attribute,range.end.y,0.17lg);
                            DUMPELFIELD(attribute,range.end.z,0.17lg);
            /*UInt16    */DUMPELFIELD(attribute,version,d);
            /*UInt16    */DUMPELFIELD(attribute,flags,d);
            /*Int32     */DUMPELFIELD(attribute,futureUse,d);
            /*DPoint3d  */DUMPELFIELD(attribute,origin.x,0.17lg);
                            DUMPELFIELD(attribute,origin.y,0.17lg);
                            DUMPELFIELD(attribute,origin.z,0.17lg);
            /*DPoint3d  */DUMPELFIELD(attribute,offset.x,0.17lg);
                            DUMPELFIELD(attribute,offset.y,0.17lg);
                            DUMPELFIELD(attribute,offset.z,0.17lg);
            /*UInt16    */DUMPELFIELD(attribute,attrDefID,d);
            /*UInt16    */DUMPELFIELD(attribute,dataType,d);
            /*UInt16    */DUMPELFIELD(attribute,unused4,d);
            /*UInt16    */DUMPELFIELD(attribute,textDrawFlags,d);
            /*double    */DUMPELFIELD(attribute,slant,0.17lg);
            /*double    */DUMPELFIELD(attribute,characterSpacing,0.17lg);
            /*double    */DUMPELFIELD(attribute,underlineSpacing,0.17lg);
            /*double    */DUMPELFIELD(attribute,lngthmult,0.17lg);
            /*double    */DUMPELFIELD(attribute,hghtmult,0.17lg);
            /*double    */DUMPELFIELD(attribute,rotquat[0],0.17lg);
                            DUMPELFIELD(attribute,rotquat[1],0.17lg);
                            DUMPELFIELD(attribute,rotquat[2],0.17lg);
                            DUMPELFIELD(attribute,rotquat[3],0.17lg);
            /*UInt32    */DUMPELFIELD(attribute,textExFlags,d);
            /*UInt32    */DUMPELFIELD(attribute,textStyleId,d);     /* style id 0 for no style */
            /*UInt32    */DUMPELFIELD(attribute,shxBigFont,d);              /* autocad compatiblity */
            /*UInt32    */DUMPELFIELD(attribute,font,d);
            /*UInt16    */DUMPELFIELD(attribute,just,d);
            /*byte      */DUMPELFIELD(attribute,unused2,x);
            /*byte      */DUMPELFIELD(attribute,nOptions,d);
            /*UInt32    */DUMPELFIELD(attribute,freezeGroup,d);
            /*UInt16    */DUMPELFIELD(attribute,dataBytes,d);
            /*UInt16    */DUMPELFIELD(attribute,newDataBytes,d);   /* to handle dataBytes > MAX_TEXT_LENGTH */
            /*UInt32    */DUMPELFIELD(attribute,iCodePage,d);
            /*UInt16    */DUMPELFIELD(attribute,value[0],d);
                            DUMPELFIELD(attribute,value[1],d);
                            DUMPELFIELD(attribute,value[2],d);
                            DUMPELFIELD(attribute,value[3],d);
            pData = (Byte*)(&attribute->value[3]+1);
            }
            break;

        case DgnV8Api::MICROSTATION_ELM:
            {
            DgnV8Api::ApplicationElm* application = (DgnV8Api::ApplicationElm*) el;
            /*UInt16    */DUMPELFIELD(application, signatureWord,d);
            pData = (Byte*)(&application->signatureWord+1);
            }
        }

    /*-----------------------------------------------------------------------------------
        Dump rest of element data in hex
    -----------------------------------------------------------------------------------*/
    Byte*   pLinkages = (Byte*)el + 2*el->ehdr.attrOffset;
    size_t  dsz = pLinkages - pData;
    if (dsz)
        {
#if defined (VERBOSE_XML_STYLE)
        fprintf(s_hashDumpFile, "<ElementData>\n");
#endif
        dumpRawBytes(pData, dsz);
#if defined (VERBOSE_XML_STYLE)
        fprintf(s_hashDumpFile, "</ElementData>\n");
#endif
        }

    /*-----------------------------------------------------------------------------------
        Dump linkages
    -----------------------------------------------------------------------------------*/
    for (auto li = veh.BeginElementLinkages(); li.IsValid(); li.ToNext())
        {
        dumpLinkage(li.GetLinkage(), li.GetData());
        }

    #if defined (VERBOSE_XML_STYLE)
        fprintf(s_hashDumpFile, "\n</Element>\n");
    #endif

    auto esz = el->ehdr.elementSize;
    if (esz < s_lowSize)
        s_lowSize = esz;
    else if (s_highSize < esz)
        s_highSize = esz;
    ++s_count;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncInfo::DumpXAttribute(DgnV8Api::ElementHandle::XAttributeIter const& ix)
    {
#if defined (DUMP_HASHES)
    fprintf(s_hashDumpFile, "XA hid:%d id:%d sz:%d\n", (int)ix.GetHandlerId().GetId(), (int)ix.GetId(), (int)ix.GetSize());
    dumpRawBytes((Byte*)ix.PeekData(), ix.GetSize());
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool SyncInfo::ComputeHash(BentleyApi::MD5& hasher, DgnV8ModelR v8Model, DgnV8Api::MSElement const& v8Element, uint32_t offsetToStartOfData)
    {
    DgnV8Api::EditElementHandle tweaked;
    tweaked.SetModelRef(&v8Model);
    m_converter._TweakElementForComparisonAndHashPurposes(tweaked, v8Element);
    DgnV8Api::MSElement const* toHash = tweaked.IsValid()? tweaked.GetElementP(): &v8Element;
    
#ifdef DUMP_HASHES
    if (s_dumpHash)
        {
        if (tweaked.IsValid())
            DumpElement(tweaked);
        else
            {
            DgnV8Api::ElementHandle v8eh(&v8Element, &v8Model);
            DumpElement(v8eh);
            }
        }
#endif

    hasher.Add((Byte*)toHash + offsetToStartOfData, (toHash->ehdr.elementSize * 2) - offsetToStartOfData);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncInfo::ComputeHash(BentleyApi::MD5& hasher, DgnV8EhCR v8eh)
    {
    auto elementData = v8eh.GetElementCP();

    Byte* data = (Byte*)elementData;

    //  ---------------------------------------------------------
    //  Header
    DgnV8Api::Elm_hdr ehdr;
    memcpy(&ehdr, &elementData->ehdr, sizeof(ehdr));
    ehdr.uniqueId = 0;
    ehdr.archive = 0;
    ehdr.lastModified = 0;
    hasher.Add((Byte*)&ehdr, sizeof(ehdr));

    data += sizeof(ehdr);

    if (ehdr.isGraphics)
        {
        DgnV8Api::Disp_hdr dhdr;
        memcpy(&dhdr, &elementData->hdr.dhdr, sizeof(dhdr));
        dhdr.grphgrp = 0; // *** WIP_CONVERTER - default converter ignores grphgrp, since there is no equivalent to graphic group in Graphite. On the other hand, custom converters might used grphgrp to construct aggregates and therefore need to know when this changes ...!
        dhdr.props.b.m = dhdr.props.b.n = 0;
        hasher.Add((Byte*)&dhdr, sizeof(dhdr));

        data += sizeof(dhdr);
        }

    uint32_t hdrSize = (uint32_t)(data - (Byte*)elementData); // compute # bytes we've hashed so far

    //  Element data
    if (!ComputeHash(hasher, *v8eh.GetDgnModelP(), *elementData, hdrSize))
        {
        uint32_t dataSize = (elementData->ehdr.elementSize*2) - hdrSize;
        hasher.Add(data, dataSize);
        }

    //  XAttributes
    for (auto xaiter = v8eh.BeginXAttributes(); xaiter.IsValid(); xaiter.ToNext())
        {
#ifdef DUMP_HASHES
        if (s_dumpHash)
            DumpXAttribute(xaiter);
#endif
        auto xahid = xaiter.GetHandlerId().GetId();
        hasher.Add((Byte*)&xahid, sizeof(xahid));
        auto xaid = xaiter.GetId();
        hasher.Add((Byte*)&xaid, sizeof(xaid));
        hasher.Add((Byte*)xaiter.PeekData(), (uint32_t)xaiter.GetSize());
        }

    for (auto xdomain : XDomainRegistry::s_xdomains)
		xdomain->_ComputeHash(hasher, v8eh);

    for (DgnV8Api::ChildElemIter child (v8eh); child.IsValid(); child=child.ToNext())
        {
        ComputeHash(hasher, child);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncInfo::SetDumpHash(FILE* fp)
    {
#ifdef DUMP_HASHES
    if (nullptr != fp)
        {
        BeAssert(nullptr == s_hashDumpFile);
        s_hashDumpFile = fp;

        s_dumpHash = true;
        s_count = 0;
        s_lowSize = UINT32_MAX;
        s_highSize = 0;
        }
    else
        {
        if (nullptr != s_hashDumpFile)
            {
            fprintf(s_hashDumpFile, "count:%d, size range:%d-%d, mean:%lf words\n", s_count, s_lowSize, s_highSize, (s_highSize-s_lowSize)/2.0);
            s_hashDumpFile = nullptr;
            }
        s_dumpHash = false;
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::ElementProvenance::ElementProvenance(DgnV8EhCR v8eh, SyncInfo& sync, StableIdPolicy policy)
    {
    m_lastModified = v8eh.GetElementCP()->ehdr.lastModified;
    m_idPolicy = policy;

    MD5 hasher;

    sync.ComputeHash(hasher, v8eh);

    m_hash = hasher.GetHashVal();

#ifdef DUMP_HASHES
    if (s_dumpHash)
        {
        fprintf(s_hashDumpFile, "lmt=%0.17lf hash=", m_lastModified);
        for (int i=0; i<MD5::HashBytes; ++i)
            fprintf(s_hashDumpFile, "%02x", m_hash.m_buffer[i]);
        fprintf(s_hashDumpFile, "\n");
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::SyncInfo(Converter& converter) : m_converter(converter), m_dgndb(nullptr)
    {
    }

