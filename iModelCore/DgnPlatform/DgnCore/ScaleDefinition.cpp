/*----------------------------------------------------------------------+
|
|   $Source: DgnCore/ScaleDefinition.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/ScaleDefinition.h>

BENTLEY_API_TYPEDEFS (BeTextFile)

static const int MAX_SCALE_NAME_LENGTH      = 0x40;
static const int MAX_SCALE_NAME_BYTES       = 2*MAX_SCALE_NAME_LENGTH;
#if defined (WIP_FOREIGN_FORMAT)
static const int MAX_SCALE_LINE_LENGTH      = 0x200;
static const int SCALE_VALUE_LENGTH         = 0x40;
#endif
static const double SCALE_CMP_EPSILON       = 0.00001;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BJB             4/89
+---------------+---------------+---------------+---------------+---------------+------*/
static double   fractionalpart (double input)
    {
    return (fabs (input - (double)(int) (input + 0.5)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BJB             4/89
+---------------+---------------+---------------+---------------+---------------+------*/
static bool mdlCnv_toRational
(
double          input,
int*            numer,
int*            denom
)
    {
    static const double fc_onehalf  = 0.5;
    static const double fc_zero     = 0.0;
    static const double fc_1        = 1.0;
    static const double fc_2        = 2.0;
    static const double fc_3        = 3.0;
    static const double fc_5        = 5.0;
    static const double fc_60       = 60.0;
    static const double fc_epsilon  = SCALE_CMP_EPSILON;

    int         reverse, negative;
    double      temp, fdenom, fnumer;

    if ((negative = (input < fc_zero)) != 0)
        input = -input;

    if ((reverse = (input > fc_1)) != 0)
        input = fc_1 / input;

    /* don't allow fractions smaller than 1/5 or numbers > 5 */
    if (input < 0.2)
        return false;

    temp = input * fc_60;

    /* see if it's an integer */
    if (fractionalpart (temp) < fc_epsilon)
        {
        fdenom = 60.0;
        fnumer = (double) (int) (temp + fc_onehalf);

        while ((fmod (fnumer, fc_2) < fc_epsilon) && (fmod (fdenom, fc_2) < fc_epsilon))
            {
            fdenom /= fc_2;
            fnumer /= fc_2;
            }

        while ((fmod (fnumer, fc_3) < fc_epsilon) && (fmod (fdenom, fc_3) < fc_epsilon))
            {
            fdenom /= fc_3;
            fnumer /= fc_3;
            }

        while ((fmod (fnumer, fc_5) < fc_epsilon) && (fmod (fdenom, fc_5) < fc_epsilon))
            {
            fdenom /= fc_5;
            fnumer /= fc_5;
            }

        if (reverse)
            {
            *numer = (int) (fdenom + fc_onehalf);
            *denom = (int) (fnumer + fc_onehalf);
            }
        else
            {
            *numer = (int) (fnumer + fc_onehalf);
            *denom = (int) (fdenom + fc_onehalf);
            }

        if (negative)
            *numer = - *numer;

        return true;
        }
    else
        {
        return false;
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          getToken                                                |
|                                                                       |
| author        MichaelSpringer                         01/02           |
|                                                                       |
+----------------------------------------------------------------------*/
#if defined (WIP_FOREIGN_FORMAT)
static bool    getToken
(
WCharP          outputP,
WCharCP*        currPosPP,
WCharCP         originalStringP,
WChar           separator
)
    {
    int         i = 0;

    if  (NULL == (*currPosPP))
        {
        (*currPosPP) = originalStringP;
        }
    else
        {
        if  ((*currPosPP)[i] == L'\0')
            {
            return  false;
            }
        if  ((*currPosPP)[i] == separator)
            {
            (*currPosPP)++;
            }
        }

    while ((*currPosPP)[i] != L'\0' && (*currPosPP)[i] != separator)
        i++;
    if  (i > 0)
        {
        wcsncpy (outputP, (*currPosPP), i);
        outputP[i] = L'\0';
        }
    else
        outputP[0] = L'\0';

    (*currPosPP) += i;
    return  true;
    }
#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 09/03
+---------------+---------------+---------------+---------------+---------------+------*/
#if defined (WIP_FOREIGN_FORMAT)
static StatusInt   getprepostscale
(
double              *prescale,
double              *postscale,
WCharCP              string
)
    {
    WCharCP pSep = NULL;
    WChar   scPreScale[SCALE_VALUE_LENGTH];

    if (NULL == prescale || NULL == postscale || NULL == string)
        return ERROR;

    *prescale = *postscale = 1.0;
    memset (scPreScale, 0, sizeof (scPreScale));

    if (NULL == (pSep = ::wcschr (string, L':')))
        return ERROR;

    BeStringUtilities::Wcsncpy (scPreScale, string, (wcslen (string) - wcslen (pSep)));

    *prescale  = BeStringUtilities::Wtof (scPreScale);
    *postscale = BeStringUtilities::Wtof (pSep+1);

    return SUCCESS;
    }
#endif
/*---------------------------------------------------------------------------------**//**
* Create named ScaleDefinition
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
ScaleDefinition::ScaleDefinition (WCharCP name, double pre, double post) : m_name(name), m_prescale(pre), m_postscale(post)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* Create empty ScaleDefinition
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+--------------+------*/
ScaleDefinition::ScaleDefinition() : m_prescale(1.0), m_postscale(1.0)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* Create custom ScaleDefinition e.g. 'CUSTOM 1.234'
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
ScaleDefinition::ScaleDefinition (double scaleFactor, bool nameAsRatio)
    {
    // Attempt to extract num/denom from scale factor
    int num, denom;
    if (mdlCnv_toRational (scaleFactor, &num, &denom))
        {
        m_prescale = (double)num;
        m_postscale = (double)denom;
        }
    else if (scaleFactor > 1.0 || scaleFactor == 0.0)
        {
        m_prescale = scaleFactor;
        m_postscale = 1.0;
        }
    else
        {
        m_prescale = 1.0;
        m_postscale = 1.0 / scaleFactor;
        }

    if (nameAsRatio)
        {
        // Generate name as "denom:num"
        FormatNameAsRatio();
        }
    else
        {
        m_name = DgnCoreL10N::GetStringW(DgnCoreL10N::SCALEDEFINITION_MSGID_SheetScaleCustom);
        if (0 == m_name.length())
            m_name = L"CUSTOM";

        WChar customStr[MAX_SCALE_NAME_BYTES];
        BeStringUtilities::Snwprintf (customStr, _countof(customStr), L" %0.5f", scaleFactor);
        wstripTrailingZeros (customStr);
        m_name.append (customStr);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ScaleDefinition::FormatNameAsRatio()
    {    
    m_name.clear();
    WChar numerString[0x20], denomString[0x20];
    BeStringUtilities::Snwprintf (numerString, _countof(numerString), L"%f", m_prescale);
    BeStringUtilities::Snwprintf (denomString, _countof(denomString), L"%f", m_postscale);
    wstripTrailingZeros (numerString);
    wstripTrailingZeros (denomString);

    m_name.Sprintf (L"%ls:%ls", denomString, numerString);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP ScaleDefinition::GetName() const
    {
    return m_name.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
double ScaleDefinition::GetScale() const
    {
    return (0.0 != m_postscale) ? m_prescale/m_postscale : 1.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ScaleDefinition::operator==(ScaleDefinition const& other) const
    {
    return SCALE_CMP_EPSILON > fabs (m_prescale - other.m_prescale)
        && SCALE_CMP_EPSILON > fabs (m_postscale - other.m_postscale)
        && m_name.Equals (other.m_name);
    }

/*---------------------------------------------------------------------------------**//**
* scales.def is parsed once and its contents cached here.
* @bsistruct                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct NamedScaleList : DgnHost::HostObjectBase
    {
private:
    bvector<ScaleDefinition>        m_scales;

    NamedScaleList();

    bool                            ParseNextScaleDef (WStringR name, double& prescale, double& postscale, BeTextFileP scalefile);

    static DgnHost::Key&            GetHostKey()        { static DgnHost::Key key; return key; }
public:
    static bvector<ScaleDefinition> const& GetScales();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ScaleDefinition> const& NamedScaleList::GetScales()
    {
    DgnHost::Key& key = GetHostKey();
    NamedScaleList* list = dynamic_cast<NamedScaleList*> (T_HOST.GetHostObject (key));
    if (NULL == list)
        {
        list = new NamedScaleList();
        T_HOST.SetHostObject (key, list);
        }

    BeAssert (NULL != list);
    return list->m_scales;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
NamedScaleList::NamedScaleList()
    {
// *** WIP_ForeignFormat - We should move this code into foreignformat.

#ifdef WIP_CFGVAR // MS_CUSTOMSCALEDEF
    // Locate and open scales.def
    WString filename;
    if (SUCCESS == ConfigurationManager::GetVariable (filename, L"MS_CUSTOMSCALEDEF") )
        {
        BeFileStatus fileOpenStatus;
        BeTextFilePtr scalefile = BeTextFile::Open (fileOpenStatus, filename.c_str(), TextFileOpenType::Read, TextFileOptions::None);
        if (BeFileStatus::Success == fileOpenStatus)
            {
            // Parse the file
            double pre, post;
            WString name;
            while (ParseNextScaleDef (name, pre, post, scalefile.get()))
                m_scales.push_back (ScaleDefinition (name.c_str(), pre, post));
            }
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool NamedScaleList::ParseNextScaleDef (WStringR name, double& prescale, double& postscale, BeTextFileP scalefile)
    {
// *** WIP_ForeignFormat - We should move this code into foreignformat.

#if defined (WIP_FOREIGN_FORMAT)
    int         status = SUCCESS;
    int         fieldNum;
    WString     fileline;
    WChar       sep = L';';
    WChar       token[MAX_SCALE_LINE_LENGTH];
    WCharCP     currPosP;
    bool        currentTokenValid;
    bool        bFoundNext = false;

    while (false == bFoundNext && TextFileReadStatus::Success == scalefile->GetLine (fileline))
        {
        if (L'#' == fileline[0])
            continue;   // comment

        for (fieldNum = 0, currPosP = NULL, currentTokenValid = getToken (token, &currPosP, fileline.c_str(), sep);
            currentTokenValid && false == bFoundNext;
            fieldNum++, currentTokenValid = getToken (token, &currPosP, fileline.c_str(), sep))
            {
            wstripSpace (token);
            switch (fieldNum)
                {
            case 0:     // name
                name = token;
                break;
            case 1:     // scale
                if (SUCCESS != (status = getprepostscale (&prescale, &postscale, token)))
                    return false;

                bFoundNext = true;
                break;
                }
            }
        }

    return bFoundNext;
#else
    BeAssert (false);
    return false;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
ScaleCollection::ScaleCollection() : m_scaleList (NamedScaleList::GetScales())
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
ScaleCollection::const_iterator ScaleCollection::begin() const
    {
    return m_scaleList.begin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
ScaleCollection::const_iterator ScaleCollection::end() const
    {
    return m_scaleList.end();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
ScaleDefinitionCP ScaleCollection::FindByName (WCharCP name) const
    {
    FOR_EACH (ScaleDefinitionCR scale, *this)
        {
        if (0 == wcscmp (scale.GetName(), name))
            return &scale;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
ScaleDefinitionCP ScaleCollection::FindByFactor (double scaleToMatch) const
    {
    FOR_EACH (ScaleDefinitionCR scale, *this)
        {
        if (SCALE_CMP_EPSILON > fabs (scale.GetScale() - scaleToMatch))
            return &scale;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* Convenience for users of old miscales api.
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ScaleDefinition::FindByFactor (ScaleDefinitionR scaleDef, double scaleFactor)
    {
    ScaleCollection scales;
    ScaleDefinitionCP foundDef = scales.FindByFactor (scaleFactor);
    scaleDef = foundDef ? *foundDef : ScaleDefinition (scaleFactor);
    }
