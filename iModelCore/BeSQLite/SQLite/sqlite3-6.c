/************** Begin file fts3_snippet.c ************************************/
/*
** 2009 Oct 23
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
*/

#if !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_FTS3)

/* #include <string.h> */
/* #include <assert.h> */

/*
** Characters that may appear in the second argument to matchinfo().
*/
#define FTS3_MATCHINFO_NPHRASE   'p'        /* 1 value */
#define FTS3_MATCHINFO_NCOL      'c'        /* 1 value */
#define FTS3_MATCHINFO_NDOC      'n'        /* 1 value */
#define FTS3_MATCHINFO_AVGLENGTH 'a'        /* nCol values */
#define FTS3_MATCHINFO_LENGTH    'l'        /* nCol values */
#define FTS3_MATCHINFO_LCS       's'        /* nCol values */
#define FTS3_MATCHINFO_HITS      'x'        /* 3*nCol*nPhrase values */

/*
** The default value for the second argument to matchinfo(). 
*/
#define FTS3_MATCHINFO_DEFAULT   "pcx"


/*
** Used as an fts3ExprIterate() context when loading phrase doclists to
** Fts3Expr.aDoclist[]/nDoclist.
*/
typedef struct LoadDoclistCtx LoadDoclistCtx;
struct LoadDoclistCtx {
  Fts3Cursor *pCsr;               /* FTS3 Cursor */
  int nPhrase;                    /* Number of phrases seen so far */
  int nToken;                     /* Number of tokens seen so far */
};

/*
** The following types are used as part of the implementation of the 
** fts3BestSnippet() routine.
*/
typedef struct SnippetIter SnippetIter;
typedef struct SnippetPhrase SnippetPhrase;
typedef struct SnippetFragment SnippetFragment;

struct SnippetIter {
  Fts3Cursor *pCsr;               /* Cursor snippet is being generated from */
  int iCol;                       /* Extract snippet from this column */
  int nSnippet;                   /* Requested snippet length (in tokens) */
  int nPhrase;                    /* Number of phrases in query */
  SnippetPhrase *aPhrase;         /* Array of size nPhrase */
  int iCurrent;                   /* First token of current snippet */
};

struct SnippetPhrase {
  int nToken;                     /* Number of tokens in phrase */
  char *pList;                    /* Pointer to start of phrase position list */
  int iHead;                      /* Next value in position list */
  char *pHead;                    /* Position list data following iHead */
  int iTail;                      /* Next value in trailing position list */
  char *pTail;                    /* Position list data following iTail */
};

struct SnippetFragment {
  int iCol;                       /* Column snippet is extracted from */
  int iPos;                       /* Index of first token in snippet */
  u64 covered;                    /* Mask of query phrases covered */
  u64 hlmask;                     /* Mask of snippet terms to highlight */
};

/*
** This type is used as an fts3ExprIterate() context object while 
** accumulating the data returned by the matchinfo() function.
*/
typedef struct MatchInfo MatchInfo;
struct MatchInfo {
  Fts3Cursor *pCursor;            /* FTS3 Cursor */
  int nCol;                       /* Number of columns in table */
  int nPhrase;                    /* Number of matchable phrases in query */
  sqlite3_int64 nDoc;             /* Number of docs in database */
  u32 *aMatchinfo;                /* Pre-allocated buffer */
};



/*
** The snippet() and offsets() functions both return text values. An instance
** of the following structure is used to accumulate those values while the
** functions are running. See fts3StringAppend() for details.
*/
typedef struct StrBuffer StrBuffer;
struct StrBuffer {
  char *z;                        /* Pointer to buffer containing string */
  int n;                          /* Length of z in bytes (excl. nul-term) */
  int nAlloc;                     /* Allocated size of buffer z in bytes */
};


/*
** This function is used to help iterate through a position-list. A position
** list is a list of unique integers, sorted from smallest to largest. Each
** element of the list is represented by an FTS3 varint that takes the value
** of the difference between the current element and the previous one plus
** two. For example, to store the position-list:
**
**     4 9 113
**
** the three varints:
**
**     6 7 106
**
** are encoded.
**
** When this function is called, *pp points to the start of an element of
** the list. *piPos contains the value of the previous entry in the list.
** After it returns, *piPos contains the value of the next element of the
** list and *pp is advanced to the following varint.
*/
static void fts3GetDeltaPosition(char **pp, int *piPos){
  int iVal;
  *pp += fts3GetVarint32(*pp, &iVal);
  *piPos += (iVal-2);
}

/*
** Helper function for fts3ExprIterate() (see below).
*/
static int fts3ExprIterate2(
  Fts3Expr *pExpr,                /* Expression to iterate phrases of */
  int *piPhrase,                  /* Pointer to phrase counter */
  int (*x)(Fts3Expr*,int,void*),  /* Callback function to invoke for phrases */
  void *pCtx                      /* Second argument to pass to callback */
){
  int rc;                         /* Return code */
  int eType = pExpr->eType;       /* Type of expression node pExpr */

  if( eType!=FTSQUERY_PHRASE ){
    assert( pExpr->pLeft && pExpr->pRight );
    rc = fts3ExprIterate2(pExpr->pLeft, piPhrase, x, pCtx);
    if( rc==SQLITE_OK && eType!=FTSQUERY_NOT ){
      rc = fts3ExprIterate2(pExpr->pRight, piPhrase, x, pCtx);
    }
  }else{
    rc = x(pExpr, *piPhrase, pCtx);
    (*piPhrase)++;
  }
  return rc;
}

/*
** Iterate through all phrase nodes in an FTS3 query, except those that
** are part of a sub-tree that is the right-hand-side of a NOT operator.
** For each phrase node found, the supplied callback function is invoked.
**
** If the callback function returns anything other than SQLITE_OK, 
** the iteration is abandoned and the error code returned immediately.
** Otherwise, SQLITE_OK is returned after a callback has been made for
** all eligible phrase nodes.
*/
static int fts3ExprIterate(
  Fts3Expr *pExpr,                /* Expression to iterate phrases of */
  int (*x)(Fts3Expr*,int,void*),  /* Callback function to invoke for phrases */
  void *pCtx                      /* Second argument to pass to callback */
){
  int iPhrase = 0;                /* Variable used as the phrase counter */
  return fts3ExprIterate2(pExpr, &iPhrase, x, pCtx);
}

/*
** This is an fts3ExprIterate() callback used while loading the doclists
** for each phrase into Fts3Expr.aDoclist[]/nDoclist. See also
** fts3ExprLoadDoclists().
*/
static int fts3ExprLoadDoclistsCb(Fts3Expr *pExpr, int iPhrase, void *ctx){
  int rc = SQLITE_OK;
  Fts3Phrase *pPhrase = pExpr->pPhrase;
  LoadDoclistCtx *p = (LoadDoclistCtx *)ctx;

  UNUSED_PARAMETER(iPhrase);

  p->nPhrase++;
  p->nToken += pPhrase->nToken;

  return rc;
}

/*
** Load the doclists for each phrase in the query associated with FTS3 cursor
** pCsr. 
**
** If pnPhrase is not NULL, then *pnPhrase is set to the number of matchable 
** phrases in the expression (all phrases except those directly or 
** indirectly descended from the right-hand-side of a NOT operator). If 
** pnToken is not NULL, then it is set to the number of tokens in all
** matchable phrases of the expression.
*/
static int fts3ExprLoadDoclists(
  Fts3Cursor *pCsr,               /* Fts3 cursor for current query */
  int *pnPhrase,                  /* OUT: Number of phrases in query */
  int *pnToken                    /* OUT: Number of tokens in query */
){
  int rc;                         /* Return Code */
  LoadDoclistCtx sCtx = {0,0,0};  /* Context for fts3ExprIterate() */
  sCtx.pCsr = pCsr;
  rc = fts3ExprIterate(pCsr->pExpr, fts3ExprLoadDoclistsCb, (void *)&sCtx);
  if( pnPhrase ) *pnPhrase = sCtx.nPhrase;
  if( pnToken ) *pnToken = sCtx.nToken;
  return rc;
}

static int fts3ExprPhraseCountCb(Fts3Expr *pExpr, int iPhrase, void *ctx){
  (*(int *)ctx)++;
  UNUSED_PARAMETER(pExpr);
  UNUSED_PARAMETER(iPhrase);
  return SQLITE_OK;
}
static int fts3ExprPhraseCount(Fts3Expr *pExpr){
  int nPhrase = 0;
  (void)fts3ExprIterate(pExpr, fts3ExprPhraseCountCb, (void *)&nPhrase);
  return nPhrase;
}

/*
** Advance the position list iterator specified by the first two 
** arguments so that it points to the first element with a value greater
** than or equal to parameter iNext.
*/
static void fts3SnippetAdvance(char **ppIter, int *piIter, int iNext){
  char *pIter = *ppIter;
  if( pIter ){
    int iIter = *piIter;

    while( iIter<iNext ){
      if( 0==(*pIter & 0xFE) ){
        iIter = -1;
        pIter = 0;
        break;
      }
      fts3GetDeltaPosition(&pIter, &iIter);
    }

    *piIter = iIter;
    *ppIter = pIter;
  }
}

/*
** Advance the snippet iterator to the next candidate snippet.
*/
static int fts3SnippetNextCandidate(SnippetIter *pIter){
  int i;                          /* Loop counter */

  if( pIter->iCurrent<0 ){
    /* The SnippetIter object has just been initialized. The first snippet
    ** candidate always starts at offset 0 (even if this candidate has a
    ** score of 0.0).
    */
    pIter->iCurrent = 0;

    /* Advance the 'head' iterator of each phrase to the first offset that
    ** is greater than or equal to (iNext+nSnippet).
    */
    for(i=0; i<pIter->nPhrase; i++){
      SnippetPhrase *pPhrase = &pIter->aPhrase[i];
      fts3SnippetAdvance(&pPhrase->pHead, &pPhrase->iHead, pIter->nSnippet);
    }
  }else{
    int iStart;
    int iEnd = 0x7FFFFFFF;

    for(i=0; i<pIter->nPhrase; i++){
      SnippetPhrase *pPhrase = &pIter->aPhrase[i];
      if( pPhrase->pHead && pPhrase->iHead<iEnd ){
        iEnd = pPhrase->iHead;
      }
    }
    if( iEnd==0x7FFFFFFF ){
      return 1;
    }

    pIter->iCurrent = iStart = iEnd - pIter->nSnippet + 1;
    for(i=0; i<pIter->nPhrase; i++){
      SnippetPhrase *pPhrase = &pIter->aPhrase[i];
      fts3SnippetAdvance(&pPhrase->pHead, &pPhrase->iHead, iEnd+1);
      fts3SnippetAdvance(&pPhrase->pTail, &pPhrase->iTail, iStart);
    }
  }

  return 0;
}

/*
** Retrieve information about the current candidate snippet of snippet 
** iterator pIter.
*/
static void fts3SnippetDetails(
  SnippetIter *pIter,             /* Snippet iterator */
  u64 mCovered,                   /* Bitmask of phrases already covered */
  int *piToken,                   /* OUT: First token of proposed snippet */
  int *piScore,                   /* OUT: "Score" for this snippet */
  u64 *pmCover,                   /* OUT: Bitmask of phrases covered */
  u64 *pmHighlight                /* OUT: Bitmask of terms to highlight */
){
  int iStart = pIter->iCurrent;   /* First token of snippet */
  int iScore = 0;                 /* Score of this snippet */
  int i;                          /* Loop counter */
  u64 mCover = 0;                 /* Mask of phrases covered by this snippet */
  u64 mHighlight = 0;             /* Mask of tokens to highlight in snippet */

  for(i=0; i<pIter->nPhrase; i++){
    SnippetPhrase *pPhrase = &pIter->aPhrase[i];
    if( pPhrase->pTail ){
      char *pCsr = pPhrase->pTail;
      int iCsr = pPhrase->iTail;

      while( iCsr<(iStart+pIter->nSnippet) ){
        int j;
        u64 mPhrase = (u64)1 << i;
        u64 mPos = (u64)1 << (iCsr - iStart);
        assert( iCsr>=iStart );
        if( (mCover|mCovered)&mPhrase ){
          iScore++;
        }else{
          iScore += 1000;
        }
        mCover |= mPhrase;

        for(j=0; j<pPhrase->nToken; j++){
          mHighlight |= (mPos>>j);
        }

        if( 0==(*pCsr & 0x0FE) ) break;
        fts3GetDeltaPosition(&pCsr, &iCsr);
      }
    }
  }

  /* Set the output variables before returning. */
  *piToken = iStart;
  *piScore = iScore;
  *pmCover = mCover;
  *pmHighlight = mHighlight;
}

/*
** This function is an fts3ExprIterate() callback used by fts3BestSnippet().
** Each invocation populates an element of the SnippetIter.aPhrase[] array.
*/
static int fts3SnippetFindPositions(Fts3Expr *pExpr, int iPhrase, void *ctx){
  SnippetIter *p = (SnippetIter *)ctx;
  SnippetPhrase *pPhrase = &p->aPhrase[iPhrase];
  char *pCsr;
  int rc;

  pPhrase->nToken = pExpr->pPhrase->nToken;
  rc = sqlite3Fts3EvalPhrasePoslist(p->pCsr, pExpr, p->iCol, &pCsr);
  assert( rc==SQLITE_OK || pCsr==0 );
  if( pCsr ){
    int iFirst = 0;
    pPhrase->pList = pCsr;
    fts3GetDeltaPosition(&pCsr, &iFirst);
    assert( iFirst>=0 );
    pPhrase->pHead = pCsr;
    pPhrase->pTail = pCsr;
    pPhrase->iHead = iFirst;
    pPhrase->iTail = iFirst;
  }else{
    assert( rc!=SQLITE_OK || (
       pPhrase->pList==0 && pPhrase->pHead==0 && pPhrase->pTail==0 
    ));
  }

  return rc;
}

/*
** Select the fragment of text consisting of nFragment contiguous tokens 
** from column iCol that represent the "best" snippet. The best snippet
** is the snippet with the highest score, where scores are calculated
** by adding:
**
**   (a) +1 point for each occurrence of a matchable phrase in the snippet.
**
**   (b) +1000 points for the first occurrence of each matchable phrase in 
**       the snippet for which the corresponding mCovered bit is not set.
**
** The selected snippet parameters are stored in structure *pFragment before
** returning. The score of the selected snippet is stored in *piScore
** before returning.
*/
static int fts3BestSnippet(
  int nSnippet,                   /* Desired snippet length */
  Fts3Cursor *pCsr,               /* Cursor to create snippet for */
  int iCol,                       /* Index of column to create snippet from */
  u64 mCovered,                   /* Mask of phrases already covered */
  u64 *pmSeen,                    /* IN/OUT: Mask of phrases seen */
  SnippetFragment *pFragment,     /* OUT: Best snippet found */
  int *piScore                    /* OUT: Score of snippet pFragment */
){
  int rc;                         /* Return Code */
  int nList;                      /* Number of phrases in expression */
  SnippetIter sIter;              /* Iterates through snippet candidates */
  int nByte;                      /* Number of bytes of space to allocate */
  int iBestScore = -1;            /* Best snippet score found so far */
  int i;                          /* Loop counter */

  memset(&sIter, 0, sizeof(sIter));

  /* Iterate through the phrases in the expression to count them. The same
  ** callback makes sure the doclists are loaded for each phrase.
  */
  rc = fts3ExprLoadDoclists(pCsr, &nList, 0);
  if( rc!=SQLITE_OK ){
    return rc;
  }

  /* Now that it is known how many phrases there are, allocate and zero
  ** the required space using malloc().
  */
  nByte = sizeof(SnippetPhrase) * nList;
  sIter.aPhrase = (SnippetPhrase *)sqlite3_malloc(nByte);
  if( !sIter.aPhrase ){
    return SQLITE_NOMEM;
  }
  memset(sIter.aPhrase, 0, nByte);

  /* Initialize the contents of the SnippetIter object. Then iterate through
  ** the set of phrases in the expression to populate the aPhrase[] array.
  */
  sIter.pCsr = pCsr;
  sIter.iCol = iCol;
  sIter.nSnippet = nSnippet;
  sIter.nPhrase = nList;
  sIter.iCurrent = -1;
  (void)fts3ExprIterate(pCsr->pExpr, fts3SnippetFindPositions, (void *)&sIter);

  /* Set the *pmSeen output variable. */
  for(i=0; i<nList; i++){
    if( sIter.aPhrase[i].pHead ){
      *pmSeen |= (u64)1 << i;
    }
  }

  /* Loop through all candidate snippets. Store the best snippet in 
  ** *pFragment. Store its associated 'score' in iBestScore.
  */
  pFragment->iCol = iCol;
  while( !fts3SnippetNextCandidate(&sIter) ){
    int iPos;
    int iScore;
    u64 mCover;
    u64 mHighlight;
    fts3SnippetDetails(&sIter, mCovered, &iPos, &iScore, &mCover, &mHighlight);
    assert( iScore>=0 );
    if( iScore>iBestScore ){
      pFragment->iPos = iPos;
      pFragment->hlmask = mHighlight;
      pFragment->covered = mCover;
      iBestScore = iScore;
    }
  }

  sqlite3_free(sIter.aPhrase);
  *piScore = iBestScore;
  return SQLITE_OK;
}


/*
** Append a string to the string-buffer passed as the first argument.
**
** If nAppend is negative, then the length of the string zAppend is
** determined using strlen().
*/
static int fts3StringAppend(
  StrBuffer *pStr,                /* Buffer to append to */
  const char *zAppend,            /* Pointer to data to append to buffer */
  int nAppend                     /* Size of zAppend in bytes (or -1) */
){
  if( nAppend<0 ){
    nAppend = (int)strlen(zAppend);
  }

  /* If there is insufficient space allocated at StrBuffer.z, use realloc()
  ** to grow the buffer until so that it is big enough to accomadate the
  ** appended data.
  */
  if( pStr->n+nAppend+1>=pStr->nAlloc ){
    int nAlloc = pStr->nAlloc+nAppend+100;
    char *zNew = sqlite3_realloc(pStr->z, nAlloc);
    if( !zNew ){
      return SQLITE_NOMEM;
    }
    pStr->z = zNew;
    pStr->nAlloc = nAlloc;
  }
  assert( pStr->z!=0 && (pStr->nAlloc >= pStr->n+nAppend+1) );

  /* Append the data to the string buffer. */
  memcpy(&pStr->z[pStr->n], zAppend, nAppend);
  pStr->n += nAppend;
  pStr->z[pStr->n] = '\0';

  return SQLITE_OK;
}

/*
** The fts3BestSnippet() function often selects snippets that end with a
** query term. That is, the final term of the snippet is always a term
** that requires highlighting. For example, if 'X' is a highlighted term
** and '.' is a non-highlighted term, BestSnippet() may select:
**
**     ........X.....X
**
** This function "shifts" the beginning of the snippet forward in the 
** document so that there are approximately the same number of 
** non-highlighted terms to the right of the final highlighted term as there
** are to the left of the first highlighted term. For example, to this:
**
**     ....X.....X....
**
** This is done as part of extracting the snippet text, not when selecting
** the snippet. Snippet selection is done based on doclists only, so there
** is no way for fts3BestSnippet() to know whether or not the document 
** actually contains terms that follow the final highlighted term. 
*/
static int fts3SnippetShift(
  Fts3Table *pTab,                /* FTS3 table snippet comes from */
  int iLangid,                    /* Language id to use in tokenizing */
  int nSnippet,                   /* Number of tokens desired for snippet */
  const char *zDoc,               /* Document text to extract snippet from */
  int nDoc,                       /* Size of buffer zDoc in bytes */
  int *piPos,                     /* IN/OUT: First token of snippet */
  u64 *pHlmask                    /* IN/OUT: Mask of tokens to highlight */
){
  u64 hlmask = *pHlmask;          /* Local copy of initial highlight-mask */

  if( hlmask ){
    int nLeft;                    /* Tokens to the left of first highlight */
    int nRight;                   /* Tokens to the right of last highlight */
    int nDesired;                 /* Ideal number of tokens to shift forward */

    for(nLeft=0; !(hlmask & ((u64)1 << nLeft)); nLeft++);
    for(nRight=0; !(hlmask & ((u64)1 << (nSnippet-1-nRight))); nRight++);
    nDesired = (nLeft-nRight)/2;

    /* Ideally, the start of the snippet should be pushed forward in the
    ** document nDesired tokens. This block checks if there are actually
    ** nDesired tokens to the right of the snippet. If so, *piPos and
    ** *pHlMask are updated to shift the snippet nDesired tokens to the
    ** right. Otherwise, the snippet is shifted by the number of tokens
    ** available.
    */
    if( nDesired>0 ){
      int nShift;                 /* Number of tokens to shift snippet by */
      int iCurrent = 0;           /* Token counter */
      int rc;                     /* Return Code */
      sqlite3_tokenizer_module *pMod;
      sqlite3_tokenizer_cursor *pC;
      pMod = (sqlite3_tokenizer_module *)pTab->pTokenizer->pModule;

      /* Open a cursor on zDoc/nDoc. Check if there are (nSnippet+nDesired)
      ** or more tokens in zDoc/nDoc.
      */
      rc = sqlite3Fts3OpenTokenizer(pTab->pTokenizer, iLangid, zDoc, nDoc, &pC);
      if( rc!=SQLITE_OK ){
        return rc;
      }
      while( rc==SQLITE_OK && iCurrent<(nSnippet+nDesired) ){
        const char *ZDUMMY; int DUMMY1 = 0, DUMMY2 = 0, DUMMY3 = 0;
        rc = pMod->xNext(pC, &ZDUMMY, &DUMMY1, &DUMMY2, &DUMMY3, &iCurrent);
      }
      pMod->xClose(pC);
      if( rc!=SQLITE_OK && rc!=SQLITE_DONE ){ return rc; }

      nShift = (rc==SQLITE_DONE)+iCurrent-nSnippet;
      assert( nShift<=nDesired );
      if( nShift>0 ){
        *piPos += nShift;
        *pHlmask = hlmask >> nShift;
      }
    }
  }
  return SQLITE_OK;
}

/*
** Extract the snippet text for fragment pFragment from cursor pCsr and
** append it to string buffer pOut.
*/
static int fts3SnippetText(
  Fts3Cursor *pCsr,               /* FTS3 Cursor */
  SnippetFragment *pFragment,     /* Snippet to extract */
  int iFragment,                  /* Fragment number */
  int isLast,                     /* True for final fragment in snippet */
  int nSnippet,                   /* Number of tokens in extracted snippet */
  const char *zOpen,              /* String inserted before highlighted term */
  const char *zClose,             /* String inserted after highlighted term */
  const char *zEllipsis,          /* String inserted between snippets */
  StrBuffer *pOut                 /* Write output here */
){
  Fts3Table *pTab = (Fts3Table *)pCsr->base.pVtab;
  int rc;                         /* Return code */
  const char *zDoc;               /* Document text to extract snippet from */
  int nDoc;                       /* Size of zDoc in bytes */
  int iCurrent = 0;               /* Current token number of document */
  int iEnd = 0;                   /* Byte offset of end of current token */
  int isShiftDone = 0;            /* True after snippet is shifted */
  int iPos = pFragment->iPos;     /* First token of snippet */
  u64 hlmask = pFragment->hlmask; /* Highlight-mask for snippet */
  int iCol = pFragment->iCol+1;   /* Query column to extract text from */
  sqlite3_tokenizer_module *pMod; /* Tokenizer module methods object */
  sqlite3_tokenizer_cursor *pC;   /* Tokenizer cursor open on zDoc/nDoc */
  
  zDoc = (const char *)sqlite3_column_text(pCsr->pStmt, iCol);
  if( zDoc==0 ){
    if( sqlite3_column_type(pCsr->pStmt, iCol)!=SQLITE_NULL ){
      return SQLITE_NOMEM;
    }
    return SQLITE_OK;
  }
  nDoc = sqlite3_column_bytes(pCsr->pStmt, iCol);

  /* Open a token cursor on the document. */
  pMod = (sqlite3_tokenizer_module *)pTab->pTokenizer->pModule;
  rc = sqlite3Fts3OpenTokenizer(pTab->pTokenizer, pCsr->iLangid, zDoc,nDoc,&pC);
  if( rc!=SQLITE_OK ){
    return rc;
  }

  while( rc==SQLITE_OK ){
    const char *ZDUMMY;           /* Dummy argument used with tokenizer */
    int DUMMY1 = -1;              /* Dummy argument used with tokenizer */
    int iBegin = 0;               /* Offset in zDoc of start of token */
    int iFin = 0;                 /* Offset in zDoc of end of token */
    int isHighlight = 0;          /* True for highlighted terms */

    /* Variable DUMMY1 is initialized to a negative value above. Elsewhere
    ** in the FTS code the variable that the third argument to xNext points to
    ** is initialized to zero before the first (*but not necessarily
    ** subsequent*) call to xNext(). This is done for a particular application
    ** that needs to know whether or not the tokenizer is being used for
    ** snippet generation or for some other purpose.
    **
    ** Extreme care is required when writing code to depend on this
    ** initialization. It is not a documented part of the tokenizer interface.
    ** If a tokenizer is used directly by any code outside of FTS, this
    ** convention might not be respected.  */
    rc = pMod->xNext(pC, &ZDUMMY, &DUMMY1, &iBegin, &iFin, &iCurrent);
    if( rc!=SQLITE_OK ){
      if( rc==SQLITE_DONE ){
        /* Special case - the last token of the snippet is also the last token
        ** of the column. Append any punctuation that occurred between the end
        ** of the previous token and the end of the document to the output. 
        ** Then break out of the loop. */
        rc = fts3StringAppend(pOut, &zDoc[iEnd], -1);
      }
      break;
    }
    if( iCurrent<iPos ){ continue; }

    if( !isShiftDone ){
      int n = nDoc - iBegin;
      rc = fts3SnippetShift(
          pTab, pCsr->iLangid, nSnippet, &zDoc[iBegin], n, &iPos, &hlmask
      );
      isShiftDone = 1;

      /* Now that the shift has been done, check if the initial "..." are
      ** required. They are required if (a) this is not the first fragment,
      ** or (b) this fragment does not begin at position 0 of its column. 
      */
      if( rc==SQLITE_OK && (iPos>0 || iFragment>0) ){
        rc = fts3StringAppend(pOut, zEllipsis, -1);
      }
      if( rc!=SQLITE_OK || iCurrent<iPos ) continue;
    }

    if( iCurrent>=(iPos+nSnippet) ){
      if( isLast ){
        rc = fts3StringAppend(pOut, zEllipsis, -1);
      }
      break;
    }

    /* Set isHighlight to true if this term should be highlighted. */
    isHighlight = (hlmask & ((u64)1 << (iCurrent-iPos)))!=0;

    if( iCurrent>iPos ) rc = fts3StringAppend(pOut, &zDoc[iEnd], iBegin-iEnd);
    if( rc==SQLITE_OK && isHighlight ) rc = fts3StringAppend(pOut, zOpen, -1);
    if( rc==SQLITE_OK ) rc = fts3StringAppend(pOut, &zDoc[iBegin], iFin-iBegin);
    if( rc==SQLITE_OK && isHighlight ) rc = fts3StringAppend(pOut, zClose, -1);

    iEnd = iFin;
  }

  pMod->xClose(pC);
  return rc;
}


/*
** This function is used to count the entries in a column-list (a 
** delta-encoded list of term offsets within a single column of a single 
** row). When this function is called, *ppCollist should point to the
** beginning of the first varint in the column-list (the varint that
** contains the position of the first matching term in the column data).
** Before returning, *ppCollist is set to point to the first byte after
** the last varint in the column-list (either the 0x00 signifying the end
** of the position-list, or the 0x01 that precedes the column number of
** the next column in the position-list).
**
** The number of elements in the column-list is returned.
*/
static int fts3ColumnlistCount(char **ppCollist){
  char *pEnd = *ppCollist;
  char c = 0;
  int nEntry = 0;

  /* A column-list is terminated by either a 0x01 or 0x00. */
  while( 0xFE & (*pEnd | c) ){
    c = *pEnd++ & 0x80;
    if( !c ) nEntry++;
  }

  *ppCollist = pEnd;
  return nEntry;
}

/*
** fts3ExprIterate() callback used to collect the "global" matchinfo stats
** for a single query. 
**
** fts3ExprIterate() callback to load the 'global' elements of a
** FTS3_MATCHINFO_HITS matchinfo array. The global stats are those elements 
** of the matchinfo array that are constant for all rows returned by the 
** current query.
**
** Argument pCtx is actually a pointer to a struct of type MatchInfo. This
** function populates Matchinfo.aMatchinfo[] as follows:
**
**   for(iCol=0; iCol<nCol; iCol++){
**     aMatchinfo[3*iPhrase*nCol + 3*iCol + 1] = X;
**     aMatchinfo[3*iPhrase*nCol + 3*iCol + 2] = Y;
**   }
**
** where X is the number of matches for phrase iPhrase is column iCol of all
** rows of the table. Y is the number of rows for which column iCol contains
** at least one instance of phrase iPhrase.
**
** If the phrase pExpr consists entirely of deferred tokens, then all X and
** Y values are set to nDoc, where nDoc is the number of documents in the 
** file system. This is done because the full-text index doclist is required
** to calculate these values properly, and the full-text index doclist is
** not available for deferred tokens.
*/
static int fts3ExprGlobalHitsCb(
  Fts3Expr *pExpr,                /* Phrase expression node */
  int iPhrase,                    /* Phrase number (numbered from zero) */
  void *pCtx                      /* Pointer to MatchInfo structure */
){
  MatchInfo *p = (MatchInfo *)pCtx;
  return sqlite3Fts3EvalPhraseStats(
      p->pCursor, pExpr, &p->aMatchinfo[3*iPhrase*p->nCol]
  );
}

/*
** fts3ExprIterate() callback used to collect the "local" part of the
** FTS3_MATCHINFO_HITS array. The local stats are those elements of the 
** array that are different for each row returned by the query.
*/
static int fts3ExprLocalHitsCb(
  Fts3Expr *pExpr,                /* Phrase expression node */
  int iPhrase,                    /* Phrase number */
  void *pCtx                      /* Pointer to MatchInfo structure */
){
  int rc = SQLITE_OK;
  MatchInfo *p = (MatchInfo *)pCtx;
  int iStart = iPhrase * p->nCol * 3;
  int i;

  for(i=0; i<p->nCol && rc==SQLITE_OK; i++){
    char *pCsr;
    rc = sqlite3Fts3EvalPhrasePoslist(p->pCursor, pExpr, i, &pCsr);
    if( pCsr ){
      p->aMatchinfo[iStart+i*3] = fts3ColumnlistCount(&pCsr);
    }else{
      p->aMatchinfo[iStart+i*3] = 0;
    }
  }

  return rc;
}

static int fts3MatchinfoCheck(
  Fts3Table *pTab, 
  char cArg,
  char **pzErr
){
  if( (cArg==FTS3_MATCHINFO_NPHRASE)
   || (cArg==FTS3_MATCHINFO_NCOL)
   || (cArg==FTS3_MATCHINFO_NDOC && pTab->bFts4)
   || (cArg==FTS3_MATCHINFO_AVGLENGTH && pTab->bFts4)
   || (cArg==FTS3_MATCHINFO_LENGTH && pTab->bHasDocsize)
   || (cArg==FTS3_MATCHINFO_LCS)
   || (cArg==FTS3_MATCHINFO_HITS)
  ){
    return SQLITE_OK;
  }
  *pzErr = sqlite3_mprintf("unrecognized matchinfo request: %c", cArg);
  return SQLITE_ERROR;
}

static int fts3MatchinfoSize(MatchInfo *pInfo, char cArg){
  int nVal;                       /* Number of integers output by cArg */

  switch( cArg ){
    case FTS3_MATCHINFO_NDOC:
    case FTS3_MATCHINFO_NPHRASE: 
    case FTS3_MATCHINFO_NCOL: 
      nVal = 1;
      break;

    case FTS3_MATCHINFO_AVGLENGTH:
    case FTS3_MATCHINFO_LENGTH:
    case FTS3_MATCHINFO_LCS:
      nVal = pInfo->nCol;
      break;

    default:
      assert( cArg==FTS3_MATCHINFO_HITS );
      nVal = pInfo->nCol * pInfo->nPhrase * 3;
      break;
  }

  return nVal;
}

static int fts3MatchinfoSelectDoctotal(
  Fts3Table *pTab,
  sqlite3_stmt **ppStmt,
  sqlite3_int64 *pnDoc,
  const char **paLen
){
  sqlite3_stmt *pStmt;
  const char *a;
  sqlite3_int64 nDoc;

  if( !*ppStmt ){
    int rc = sqlite3Fts3SelectDoctotal(pTab, ppStmt);
    if( rc!=SQLITE_OK ) return rc;
  }
  pStmt = *ppStmt;
  assert( sqlite3_data_count(pStmt)==1 );

  a = sqlite3_column_blob(pStmt, 0);
  a += sqlite3Fts3GetVarint(a, &nDoc);
  if( nDoc==0 ) return FTS_CORRUPT_VTAB;
  *pnDoc = (u32)nDoc;

  if( paLen ) *paLen = a;
  return SQLITE_OK;
}

/*
** An instance of the following structure is used to store state while 
** iterating through a multi-column position-list corresponding to the
** hits for a single phrase on a single row in order to calculate the
** values for a matchinfo() FTS3_MATCHINFO_LCS request.
*/
typedef struct LcsIterator LcsIterator;
struct LcsIterator {
  Fts3Expr *pExpr;                /* Pointer to phrase expression */
  int iPosOffset;                 /* Tokens count up to end of this phrase */
  char *pRead;                    /* Cursor used to iterate through aDoclist */
  int iPos;                       /* Current position */
};

/* 
** If LcsIterator.iCol is set to the following value, the iterator has
** finished iterating through all offsets for all columns.
*/
#define LCS_ITERATOR_FINISHED 0x7FFFFFFF;

static int fts3MatchinfoLcsCb(
  Fts3Expr *pExpr,                /* Phrase expression node */
  int iPhrase,                    /* Phrase number (numbered from zero) */
  void *pCtx                      /* Pointer to MatchInfo structure */
){
  LcsIterator *aIter = (LcsIterator *)pCtx;
  aIter[iPhrase].pExpr = pExpr;
  return SQLITE_OK;
}

/*
** Advance the iterator passed as an argument to the next position. Return
** 1 if the iterator is at EOF or if it now points to the start of the
** position list for the next column.
*/
static int fts3LcsIteratorAdvance(LcsIterator *pIter){
  char *pRead = pIter->pRead;
  sqlite3_int64 iRead;
  int rc = 0;

  pRead += sqlite3Fts3GetVarint(pRead, &iRead);
  if( iRead==0 || iRead==1 ){
    pRead = 0;
    rc = 1;
  }else{
    pIter->iPos += (int)(iRead-2);
  }

  pIter->pRead = pRead;
  return rc;
}
  
/*
** This function implements the FTS3_MATCHINFO_LCS matchinfo() flag. 
**
** If the call is successful, the longest-common-substring lengths for each
** column are written into the first nCol elements of the pInfo->aMatchinfo[] 
** array before returning. SQLITE_OK is returned in this case.
**
** Otherwise, if an error occurs, an SQLite error code is returned and the
** data written to the first nCol elements of pInfo->aMatchinfo[] is 
** undefined.
*/
static int fts3MatchinfoLcs(Fts3Cursor *pCsr, MatchInfo *pInfo){
  LcsIterator *aIter;
  int i;
  int iCol;
  int nToken = 0;

  /* Allocate and populate the array of LcsIterator objects. The array
  ** contains one element for each matchable phrase in the query.
  **/
  aIter = sqlite3_malloc(sizeof(LcsIterator) * pCsr->nPhrase);
  if( !aIter ) return SQLITE_NOMEM;
  memset(aIter, 0, sizeof(LcsIterator) * pCsr->nPhrase);
  (void)fts3ExprIterate(pCsr->pExpr, fts3MatchinfoLcsCb, (void*)aIter);

  for(i=0; i<pInfo->nPhrase; i++){
    LcsIterator *pIter = &aIter[i];
    nToken -= pIter->pExpr->pPhrase->nToken;
    pIter->iPosOffset = nToken;
  }

  for(iCol=0; iCol<pInfo->nCol; iCol++){
    int nLcs = 0;                 /* LCS value for this column */
    int nLive = 0;                /* Number of iterators in aIter not at EOF */

    for(i=0; i<pInfo->nPhrase; i++){
      int rc;
      LcsIterator *pIt = &aIter[i];
      rc = sqlite3Fts3EvalPhrasePoslist(pCsr, pIt->pExpr, iCol, &pIt->pRead);
      if( rc!=SQLITE_OK ) return rc;
      if( pIt->pRead ){
        pIt->iPos = pIt->iPosOffset;
        fts3LcsIteratorAdvance(&aIter[i]);
        nLive++;
      }
    }

    while( nLive>0 ){
      LcsIterator *pAdv = 0;      /* The iterator to advance by one position */
      int nThisLcs = 0;           /* LCS for the current iterator positions */

      for(i=0; i<pInfo->nPhrase; i++){
        LcsIterator *pIter = &aIter[i];
        if( pIter->pRead==0 ){
          /* This iterator is already at EOF for this column. */
          nThisLcs = 0;
        }else{
          if( pAdv==0 || pIter->iPos<pAdv->iPos ){
            pAdv = pIter;
          }
          if( nThisLcs==0 || pIter->iPos==pIter[-1].iPos ){
            nThisLcs++;
          }else{
            nThisLcs = 1;
          }
          if( nThisLcs>nLcs ) nLcs = nThisLcs;
        }
      }
      if( fts3LcsIteratorAdvance(pAdv) ) nLive--;
    }

    pInfo->aMatchinfo[iCol] = nLcs;
  }

  sqlite3_free(aIter);
  return SQLITE_OK;
}

/*
** Populate the buffer pInfo->aMatchinfo[] with an array of integers to
** be returned by the matchinfo() function. Argument zArg contains the 
** format string passed as the second argument to matchinfo (or the
** default value "pcx" if no second argument was specified). The format
** string has already been validated and the pInfo->aMatchinfo[] array
** is guaranteed to be large enough for the output.
**
** If bGlobal is true, then populate all fields of the matchinfo() output.
** If it is false, then assume that those fields that do not change between
** rows (i.e. FTS3_MATCHINFO_NPHRASE, NCOL, NDOC, AVGLENGTH and part of HITS)
** have already been populated.
**
** Return SQLITE_OK if successful, or an SQLite error code if an error 
** occurs. If a value other than SQLITE_OK is returned, the state the
** pInfo->aMatchinfo[] buffer is left in is undefined.
*/
static int fts3MatchinfoValues(
  Fts3Cursor *pCsr,               /* FTS3 cursor object */
  int bGlobal,                    /* True to grab the global stats */
  MatchInfo *pInfo,               /* Matchinfo context object */
  const char *zArg                /* Matchinfo format string */
){
  int rc = SQLITE_OK;
  int i;
  Fts3Table *pTab = (Fts3Table *)pCsr->base.pVtab;
  sqlite3_stmt *pSelect = 0;

  for(i=0; rc==SQLITE_OK && zArg[i]; i++){

    switch( zArg[i] ){
      case FTS3_MATCHINFO_NPHRASE:
        if( bGlobal ) pInfo->aMatchinfo[0] = pInfo->nPhrase;
        break;

      case FTS3_MATCHINFO_NCOL:
        if( bGlobal ) pInfo->aMatchinfo[0] = pInfo->nCol;
        break;
        
      case FTS3_MATCHINFO_NDOC:
        if( bGlobal ){
          sqlite3_int64 nDoc = 0;
          rc = fts3MatchinfoSelectDoctotal(pTab, &pSelect, &nDoc, 0);
          pInfo->aMatchinfo[0] = (u32)nDoc;
        }
        break;

      case FTS3_MATCHINFO_AVGLENGTH: 
        if( bGlobal ){
          sqlite3_int64 nDoc;     /* Number of rows in table */
          const char *a;          /* Aggregate column length array */

          rc = fts3MatchinfoSelectDoctotal(pTab, &pSelect, &nDoc, &a);
          if( rc==SQLITE_OK ){
            int iCol;
            for(iCol=0; iCol<pInfo->nCol; iCol++){
              u32 iVal;
              sqlite3_int64 nToken;
              a += sqlite3Fts3GetVarint(a, &nToken);
              iVal = (u32)(((u32)(nToken&0xffffffff)+nDoc/2)/nDoc);
              pInfo->aMatchinfo[iCol] = iVal;
            }
          }
        }
        break;

      case FTS3_MATCHINFO_LENGTH: {
        sqlite3_stmt *pSelectDocsize = 0;
        rc = sqlite3Fts3SelectDocsize(pTab, pCsr->iPrevId, &pSelectDocsize);
        if( rc==SQLITE_OK ){
          int iCol;
          const char *a = sqlite3_column_blob(pSelectDocsize, 0);
          for(iCol=0; iCol<pInfo->nCol; iCol++){
            sqlite3_int64 nToken;
            a += sqlite3Fts3GetVarint(a, &nToken);
            pInfo->aMatchinfo[iCol] = (u32)nToken;
          }
        }
        sqlite3_reset(pSelectDocsize);
        break;
      }

      case FTS3_MATCHINFO_LCS:
        rc = fts3ExprLoadDoclists(pCsr, 0, 0);
        if( rc==SQLITE_OK ){
          rc = fts3MatchinfoLcs(pCsr, pInfo);
        }
        break;

      default: {
        Fts3Expr *pExpr;
        assert( zArg[i]==FTS3_MATCHINFO_HITS );
        pExpr = pCsr->pExpr;
        rc = fts3ExprLoadDoclists(pCsr, 0, 0);
        if( rc!=SQLITE_OK ) break;
        if( bGlobal ){
          if( pCsr->pDeferred ){
            rc = fts3MatchinfoSelectDoctotal(pTab, &pSelect, &pInfo->nDoc, 0);
            if( rc!=SQLITE_OK ) break;
          }
          rc = fts3ExprIterate(pExpr, fts3ExprGlobalHitsCb,(void*)pInfo);
          if( rc!=SQLITE_OK ) break;
        }
        (void)fts3ExprIterate(pExpr, fts3ExprLocalHitsCb,(void*)pInfo);
        break;
      }
    }

    pInfo->aMatchinfo += fts3MatchinfoSize(pInfo, zArg[i]);
  }

  sqlite3_reset(pSelect);
  return rc;
}


/*
** Populate pCsr->aMatchinfo[] with data for the current row. The 
** 'matchinfo' data is an array of 32-bit unsigned integers (C type u32).
*/
static int fts3GetMatchinfo(
  Fts3Cursor *pCsr,               /* FTS3 Cursor object */
  const char *zArg                /* Second argument to matchinfo() function */
){
  MatchInfo sInfo;
  Fts3Table *pTab = (Fts3Table *)pCsr->base.pVtab;
  int rc = SQLITE_OK;
  int bGlobal = 0;                /* Collect 'global' stats as well as local */

  memset(&sInfo, 0, sizeof(MatchInfo));
  sInfo.pCursor = pCsr;
  sInfo.nCol = pTab->nColumn;

  /* If there is cached matchinfo() data, but the format string for the 
  ** cache does not match the format string for this request, discard 
  ** the cached data. */
  if( pCsr->zMatchinfo && strcmp(pCsr->zMatchinfo, zArg) ){
    assert( pCsr->aMatchinfo );
    sqlite3_free(pCsr->aMatchinfo);
    pCsr->zMatchinfo = 0;
    pCsr->aMatchinfo = 0;
  }

  /* If Fts3Cursor.aMatchinfo[] is NULL, then this is the first time the
  ** matchinfo function has been called for this query. In this case 
  ** allocate the array used to accumulate the matchinfo data and
  ** initialize those elements that are constant for every row.
  */
  if( pCsr->aMatchinfo==0 ){
    int nMatchinfo = 0;           /* Number of u32 elements in match-info */
    int nArg;                     /* Bytes in zArg */
    int i;                        /* Used to iterate through zArg */

    /* Determine the number of phrases in the query */
    pCsr->nPhrase = fts3ExprPhraseCount(pCsr->pExpr);
    sInfo.nPhrase = pCsr->nPhrase;

    /* Determine the number of integers in the buffer returned by this call. */
    for(i=0; zArg[i]; i++){
      nMatchinfo += fts3MatchinfoSize(&sInfo, zArg[i]);
    }

    /* Allocate space for Fts3Cursor.aMatchinfo[] and Fts3Cursor.zMatchinfo. */
    nArg = (int)strlen(zArg);
    pCsr->aMatchinfo = (u32 *)sqlite3_malloc(sizeof(u32)*nMatchinfo + nArg + 1);
    if( !pCsr->aMatchinfo ) return SQLITE_NOMEM;

    pCsr->zMatchinfo = (char *)&pCsr->aMatchinfo[nMatchinfo];
    pCsr->nMatchinfo = nMatchinfo;
    memcpy(pCsr->zMatchinfo, zArg, nArg+1);
    memset(pCsr->aMatchinfo, 0, sizeof(u32)*nMatchinfo);
    pCsr->isMatchinfoNeeded = 1;
    bGlobal = 1;
  }

  sInfo.aMatchinfo = pCsr->aMatchinfo;
  sInfo.nPhrase = pCsr->nPhrase;
  if( pCsr->isMatchinfoNeeded ){
    rc = fts3MatchinfoValues(pCsr, bGlobal, &sInfo, zArg);
    pCsr->isMatchinfoNeeded = 0;
  }

  return rc;
}

/*
** Implementation of snippet() function.
*/
SQLITE_PRIVATE void sqlite3Fts3Snippet(
  sqlite3_context *pCtx,          /* SQLite function call context */
  Fts3Cursor *pCsr,               /* Cursor object */
  const char *zStart,             /* Snippet start text - "<b>" */
  const char *zEnd,               /* Snippet end text - "</b>" */
  const char *zEllipsis,          /* Snippet ellipsis text - "<b>...</b>" */
  int iCol,                       /* Extract snippet from this column */
  int nToken                      /* Approximate number of tokens in snippet */
){
  Fts3Table *pTab = (Fts3Table *)pCsr->base.pVtab;
  int rc = SQLITE_OK;
  int i;
  StrBuffer res = {0, 0, 0};

  /* The returned text includes up to four fragments of text extracted from
  ** the data in the current row. The first iteration of the for(...) loop
  ** below attempts to locate a single fragment of text nToken tokens in 
  ** size that contains at least one instance of all phrases in the query
  ** expression that appear in the current row. If such a fragment of text
  ** cannot be found, the second iteration of the loop attempts to locate
  ** a pair of fragments, and so on.
  */
  int nSnippet = 0;               /* Number of fragments in this snippet */
  SnippetFragment aSnippet[4];    /* Maximum of 4 fragments per snippet */
  int nFToken = -1;               /* Number of tokens in each fragment */

  if( !pCsr->pExpr ){
    sqlite3_result_text(pCtx, "", 0, SQLITE_STATIC);
    return;
  }

  for(nSnippet=1; 1; nSnippet++){

    int iSnip;                    /* Loop counter 0..nSnippet-1 */
    u64 mCovered = 0;             /* Bitmask of phrases covered by snippet */
    u64 mSeen = 0;                /* Bitmask of phrases seen by BestSnippet() */

    if( nToken>=0 ){
      nFToken = (nToken+nSnippet-1) / nSnippet;
    }else{
      nFToken = -1 * nToken;
    }

    for(iSnip=0; iSnip<nSnippet; iSnip++){
      int iBestScore = -1;        /* Best score of columns checked so far */
      int iRead;                  /* Used to iterate through columns */
      SnippetFragment *pFragment = &aSnippet[iSnip];

      memset(pFragment, 0, sizeof(*pFragment));

      /* Loop through all columns of the table being considered for snippets.
      ** If the iCol argument to this function was negative, this means all
      ** columns of the FTS3 table. Otherwise, only column iCol is considered.
      */
      for(iRead=0; iRead<pTab->nColumn; iRead++){
        SnippetFragment sF = {0, 0, 0, 0};
        int iS;
        if( iCol>=0 && iRead!=iCol ) continue;

        /* Find the best snippet of nFToken tokens in column iRead. */
        rc = fts3BestSnippet(nFToken, pCsr, iRead, mCovered, &mSeen, &sF, &iS);
        if( rc!=SQLITE_OK ){
          goto snippet_out;
        }
        if( iS>iBestScore ){
          *pFragment = sF;
          iBestScore = iS;
        }
      }

      mCovered |= pFragment->covered;
    }

    /* If all query phrases seen by fts3BestSnippet() are present in at least
    ** one of the nSnippet snippet fragments, break out of the loop.
    */
    assert( (mCovered&mSeen)==mCovered );
    if( mSeen==mCovered || nSnippet==SizeofArray(aSnippet) ) break;
  }

  assert( nFToken>0 );

  for(i=0; i<nSnippet && rc==SQLITE_OK; i++){
    rc = fts3SnippetText(pCsr, &aSnippet[i], 
        i, (i==nSnippet-1), nFToken, zStart, zEnd, zEllipsis, &res
    );
  }

 snippet_out:
  sqlite3Fts3SegmentsClose(pTab);
  if( rc!=SQLITE_OK ){
    sqlite3_result_error_code(pCtx, rc);
    sqlite3_free(res.z);
  }else{
    sqlite3_result_text(pCtx, res.z, -1, sqlite3_free);
  }
}


typedef struct TermOffset TermOffset;
typedef struct TermOffsetCtx TermOffsetCtx;

struct TermOffset {
  char *pList;                    /* Position-list */
  int iPos;                       /* Position just read from pList */
  int iOff;                       /* Offset of this term from read positions */
};

struct TermOffsetCtx {
  Fts3Cursor *pCsr;
  int iCol;                       /* Column of table to populate aTerm for */
  int iTerm;
  sqlite3_int64 iDocid;
  TermOffset *aTerm;
};

/*
** This function is an fts3ExprIterate() callback used by sqlite3Fts3Offsets().
*/
static int fts3ExprTermOffsetInit(Fts3Expr *pExpr, int iPhrase, void *ctx){
  TermOffsetCtx *p = (TermOffsetCtx *)ctx;
  int nTerm;                      /* Number of tokens in phrase */
  int iTerm;                      /* For looping through nTerm phrase terms */
  char *pList;                    /* Pointer to position list for phrase */
  int iPos = 0;                   /* First position in position-list */
  int rc;

  UNUSED_PARAMETER(iPhrase);
  rc = sqlite3Fts3EvalPhrasePoslist(p->pCsr, pExpr, p->iCol, &pList);
  nTerm = pExpr->pPhrase->nToken;
  if( pList ){
    fts3GetDeltaPosition(&pList, &iPos);
    assert( iPos>=0 );
  }

  for(iTerm=0; iTerm<nTerm; iTerm++){
    TermOffset *pT = &p->aTerm[p->iTerm++];
    pT->iOff = nTerm-iTerm-1;
    pT->pList = pList;
    pT->iPos = iPos;
  }

  return rc;
}

/*
** Implementation of offsets() function.
*/
SQLITE_PRIVATE void sqlite3Fts3Offsets(
  sqlite3_context *pCtx,          /* SQLite function call context */
  Fts3Cursor *pCsr                /* Cursor object */
){
  Fts3Table *pTab = (Fts3Table *)pCsr->base.pVtab;
  sqlite3_tokenizer_module const *pMod = pTab->pTokenizer->pModule;
  int rc;                         /* Return Code */
  int nToken;                     /* Number of tokens in query */
  int iCol;                       /* Column currently being processed */
  StrBuffer res = {0, 0, 0};      /* Result string */
  TermOffsetCtx sCtx;             /* Context for fts3ExprTermOffsetInit() */

  if( !pCsr->pExpr ){
    sqlite3_result_text(pCtx, "", 0, SQLITE_STATIC);
    return;
  }

  memset(&sCtx, 0, sizeof(sCtx));
  assert( pCsr->isRequireSeek==0 );

  /* Count the number of terms in the query */
  rc = fts3ExprLoadDoclists(pCsr, 0, &nToken);
  if( rc!=SQLITE_OK ) goto offsets_out;

  /* Allocate the array of TermOffset iterators. */
  sCtx.aTerm = (TermOffset *)sqlite3_malloc(sizeof(TermOffset)*nToken);
  if( 0==sCtx.aTerm ){
    rc = SQLITE_NOMEM;
    goto offsets_out;
  }
  sCtx.iDocid = pCsr->iPrevId;
  sCtx.pCsr = pCsr;

  /* Loop through the table columns, appending offset information to 
  ** string-buffer res for each column.
  */
  for(iCol=0; iCol<pTab->nColumn; iCol++){
    sqlite3_tokenizer_cursor *pC; /* Tokenizer cursor */
    const char *ZDUMMY;           /* Dummy argument used with xNext() */
    int NDUMMY = 0;               /* Dummy argument used with xNext() */
    int iStart = 0;
    int iEnd = 0;
    int iCurrent = 0;
    const char *zDoc;
    int nDoc;

    /* Initialize the contents of sCtx.aTerm[] for column iCol. There is 
    ** no way that this operation can fail, so the return code from
    ** fts3ExprIterate() can be discarded.
    */
    sCtx.iCol = iCol;
    sCtx.iTerm = 0;
    (void)fts3ExprIterate(pCsr->pExpr, fts3ExprTermOffsetInit, (void *)&sCtx);

    /* Retreive the text stored in column iCol. If an SQL NULL is stored 
    ** in column iCol, jump immediately to the next iteration of the loop.
    ** If an OOM occurs while retrieving the data (this can happen if SQLite
    ** needs to transform the data from utf-16 to utf-8), return SQLITE_NOMEM 
    ** to the caller. 
    */
    zDoc = (const char *)sqlite3_column_text(pCsr->pStmt, iCol+1);
    nDoc = sqlite3_column_bytes(pCsr->pStmt, iCol+1);
    if( zDoc==0 ){
      if( sqlite3_column_type(pCsr->pStmt, iCol+1)==SQLITE_NULL ){
        continue;
      }
      rc = SQLITE_NOMEM;
      goto offsets_out;
    }

    /* Initialize a tokenizer iterator to iterate through column iCol. */
    rc = sqlite3Fts3OpenTokenizer(pTab->pTokenizer, pCsr->iLangid,
        zDoc, nDoc, &pC
    );
    if( rc!=SQLITE_OK ) goto offsets_out;

    rc = pMod->xNext(pC, &ZDUMMY, &NDUMMY, &iStart, &iEnd, &iCurrent);
    while( rc==SQLITE_OK ){
      int i;                      /* Used to loop through terms */
      int iMinPos = 0x7FFFFFFF;   /* Position of next token */
      TermOffset *pTerm = 0;      /* TermOffset associated with next token */

      for(i=0; i<nToken; i++){
        TermOffset *pT = &sCtx.aTerm[i];
        if( pT->pList && (pT->iPos-pT->iOff)<iMinPos ){
          iMinPos = pT->iPos-pT->iOff;
          pTerm = pT;
        }
      }

      if( !pTerm ){
        /* All offsets for this column have been gathered. */
        rc = SQLITE_DONE;
      }else{
        assert( iCurrent<=iMinPos );
        if( 0==(0xFE&*pTerm->pList) ){
          pTerm->pList = 0;
        }else{
          fts3GetDeltaPosition(&pTerm->pList, &pTerm->iPos);
        }
        while( rc==SQLITE_OK && iCurrent<iMinPos ){
          rc = pMod->xNext(pC, &ZDUMMY, &NDUMMY, &iStart, &iEnd, &iCurrent);
        }
        if( rc==SQLITE_OK ){
          char aBuffer[64];
          sqlite3_snprintf(sizeof(aBuffer), aBuffer, 
              "%d %d %d %d ", iCol, pTerm-sCtx.aTerm, iStart, iEnd-iStart
          );
          rc = fts3StringAppend(&res, aBuffer, -1);
        }else if( rc==SQLITE_DONE && pTab->zContentTbl==0 ){
          rc = FTS_CORRUPT_VTAB;
        }
      }
    }
    if( rc==SQLITE_DONE ){
      rc = SQLITE_OK;
    }

    pMod->xClose(pC);
    if( rc!=SQLITE_OK ) goto offsets_out;
  }

 offsets_out:
  sqlite3_free(sCtx.aTerm);
  assert( rc!=SQLITE_DONE );
  sqlite3Fts3SegmentsClose(pTab);
  if( rc!=SQLITE_OK ){
    sqlite3_result_error_code(pCtx,  rc);
    sqlite3_free(res.z);
  }else{
    sqlite3_result_text(pCtx, res.z, res.n-1, sqlite3_free);
  }
  return;
}

/*
** Implementation of matchinfo() function.
*/
SQLITE_PRIVATE void sqlite3Fts3Matchinfo(
  sqlite3_context *pContext,      /* Function call context */
  Fts3Cursor *pCsr,               /* FTS3 table cursor */
  const char *zArg                /* Second arg to matchinfo() function */
){
  Fts3Table *pTab = (Fts3Table *)pCsr->base.pVtab;
  int rc;
  int i;
  const char *zFormat;

  if( zArg ){
    for(i=0; zArg[i]; i++){
      char *zErr = 0;
      if( fts3MatchinfoCheck(pTab, zArg[i], &zErr) ){
        sqlite3_result_error(pContext, zErr, -1);
        sqlite3_free(zErr);
        return;
      }
    }
    zFormat = zArg;
  }else{
    zFormat = FTS3_MATCHINFO_DEFAULT;
  }

  if( !pCsr->pExpr ){
    sqlite3_result_blob(pContext, "", 0, SQLITE_STATIC);
    return;
  }

  /* Retrieve matchinfo() data. */
  rc = fts3GetMatchinfo(pCsr, zFormat);
  sqlite3Fts3SegmentsClose(pTab);

  if( rc!=SQLITE_OK ){
    sqlite3_result_error_code(pContext, rc);
  }else{
    int n = pCsr->nMatchinfo * sizeof(u32);
    sqlite3_result_blob(pContext, pCsr->aMatchinfo, n, SQLITE_TRANSIENT);
  }
}

#endif

/************** End of fts3_snippet.c ****************************************/
/************** Begin file fts3_unicode.c ************************************/
/*
** 2012 May 24
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** Implementation of the "unicode" full-text-search tokenizer.
*/

#ifndef SQLITE_DISABLE_FTS3_UNICODE

#if !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_FTS3)

/* #include <assert.h> */
/* #include <stdlib.h> */
/* #include <stdio.h> */
/* #include <string.h> */


/*
** The following two macros - READ_UTF8 and WRITE_UTF8 - have been copied
** from the sqlite3 source file utf.c. If this file is compiled as part
** of the amalgamation, they are not required.
*/
#ifndef SQLITE_AMALGAMATION

static const unsigned char sqlite3Utf8Trans1[] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x00, 0x00,
};

#define READ_UTF8(zIn, zTerm, c)                           \
  c = *(zIn++);                                            \
  if( c>=0xc0 ){                                           \
    c = sqlite3Utf8Trans1[c-0xc0];                         \
    while( zIn!=zTerm && (*zIn & 0xc0)==0x80 ){            \
      c = (c<<6) + (0x3f & *(zIn++));                      \
    }                                                      \
    if( c<0x80                                             \
        || (c&0xFFFFF800)==0xD800                          \
        || (c&0xFFFFFFFE)==0xFFFE ){  c = 0xFFFD; }        \
  }

#define WRITE_UTF8(zOut, c) {                          \
  if( c<0x00080 ){                                     \
    *zOut++ = (u8)(c&0xFF);                            \
  }                                                    \
  else if( c<0x00800 ){                                \
    *zOut++ = 0xC0 + (u8)((c>>6)&0x1F);                \
    *zOut++ = 0x80 + (u8)(c & 0x3F);                   \
  }                                                    \
  else if( c<0x10000 ){                                \
    *zOut++ = 0xE0 + (u8)((c>>12)&0x0F);               \
    *zOut++ = 0x80 + (u8)((c>>6) & 0x3F);              \
    *zOut++ = 0x80 + (u8)(c & 0x3F);                   \
  }else{                                               \
    *zOut++ = 0xF0 + (u8)((c>>18) & 0x07);             \
    *zOut++ = 0x80 + (u8)((c>>12) & 0x3F);             \
    *zOut++ = 0x80 + (u8)((c>>6) & 0x3F);              \
    *zOut++ = 0x80 + (u8)(c & 0x3F);                   \
  }                                                    \
}

#endif /* ifndef SQLITE_AMALGAMATION */

typedef struct unicode_tokenizer unicode_tokenizer;
typedef struct unicode_cursor unicode_cursor;

struct unicode_tokenizer {
  sqlite3_tokenizer base;
  int bRemoveDiacritic;
  int nException;
  int *aiException;
};

struct unicode_cursor {
  sqlite3_tokenizer_cursor base;
  const unsigned char *aInput;    /* Input text being tokenized */
  int nInput;                     /* Size of aInput[] in bytes */
  int iOff;                       /* Current offset within aInput[] */
  int iToken;                     /* Index of next token to be returned */
  char *zToken;                   /* storage for current token */
  int nAlloc;                     /* space allocated at zToken */
};


/*
** Destroy a tokenizer allocated by unicodeCreate().
*/
static int unicodeDestroy(sqlite3_tokenizer *pTokenizer){
  if( pTokenizer ){
    unicode_tokenizer *p = (unicode_tokenizer *)pTokenizer;
    sqlite3_free(p->aiException);
    sqlite3_free(p);
  }
  return SQLITE_OK;
}

/*
** As part of a tokenchars= or separators= option, the CREATE VIRTUAL TABLE
** statement has specified that the tokenizer for this table shall consider
** all characters in string zIn/nIn to be separators (if bAlnum==0) or
** token characters (if bAlnum==1).
**
** For each codepoint in the zIn/nIn string, this function checks if the
** sqlite3FtsUnicodeIsalnum() function already returns the desired result.
** If so, no action is taken. Otherwise, the codepoint is added to the 
** unicode_tokenizer.aiException[] array. For the purposes of tokenization,
** the return value of sqlite3FtsUnicodeIsalnum() is inverted for all
** codepoints in the aiException[] array.
**
** If a standalone diacritic mark (one that sqlite3FtsUnicodeIsdiacritic()
** identifies as a diacritic) occurs in the zIn/nIn string it is ignored.
** It is not possible to change the behavior of the tokenizer with respect
** to these codepoints.
*/
static int unicodeAddExceptions(
  unicode_tokenizer *p,           /* Tokenizer to add exceptions to */
  int bAlnum,                     /* Replace Isalnum() return value with this */
  const char *zIn,                /* Array of characters to make exceptions */
  int nIn                         /* Length of z in bytes */
){
  const unsigned char *z = (const unsigned char *)zIn;
  const unsigned char *zTerm = &z[nIn];
  int iCode;
  int nEntry = 0;

  assert( bAlnum==0 || bAlnum==1 );

  while( z<zTerm ){
    READ_UTF8(z, zTerm, iCode);
    assert( (sqlite3FtsUnicodeIsalnum(iCode) & 0xFFFFFFFE)==0 );
    if( sqlite3FtsUnicodeIsalnum(iCode)!=bAlnum 
     && sqlite3FtsUnicodeIsdiacritic(iCode)==0 
    ){
      nEntry++;
    }
  }

  if( nEntry ){
    int *aNew;                    /* New aiException[] array */
    int nNew;                     /* Number of valid entries in array aNew[] */

    aNew = sqlite3_realloc(p->aiException, (p->nException+nEntry)*sizeof(int));
    if( aNew==0 ) return SQLITE_NOMEM;
    nNew = p->nException;

    z = (const unsigned char *)zIn;
    while( z<zTerm ){
      READ_UTF8(z, zTerm, iCode);
      if( sqlite3FtsUnicodeIsalnum(iCode)!=bAlnum 
       && sqlite3FtsUnicodeIsdiacritic(iCode)==0
      ){
        int i, j;
        for(i=0; i<nNew && aNew[i]<iCode; i++);
        for(j=nNew; j>i; j--) aNew[j] = aNew[j-1];
        aNew[i] = iCode;
        nNew++;
      }
    }
    p->aiException = aNew;
    p->nException = nNew;
  }

  return SQLITE_OK;
}

/*
** Return true if the p->aiException[] array contains the value iCode.
*/
static int unicodeIsException(unicode_tokenizer *p, int iCode){
  if( p->nException>0 ){
    int *a = p->aiException;
    int iLo = 0;
    int iHi = p->nException-1;

    while( iHi>=iLo ){
      int iTest = (iHi + iLo) / 2;
      if( iCode==a[iTest] ){
        return 1;
      }else if( iCode>a[iTest] ){
        iLo = iTest+1;
      }else{
        iHi = iTest-1;
      }
    }
  }

  return 0;
}

/*
** Return true if, for the purposes of tokenization, codepoint iCode is
** considered a token character (not a separator).
*/
static int unicodeIsAlnum(unicode_tokenizer *p, int iCode){
  assert( (sqlite3FtsUnicodeIsalnum(iCode) & 0xFFFFFFFE)==0 );
  return sqlite3FtsUnicodeIsalnum(iCode) ^ unicodeIsException(p, iCode);
}

/*
** Create a new tokenizer instance.
*/
static int unicodeCreate(
  int nArg,                       /* Size of array argv[] */
  const char * const *azArg,      /* Tokenizer creation arguments */
  sqlite3_tokenizer **pp          /* OUT: New tokenizer handle */
){
  unicode_tokenizer *pNew;        /* New tokenizer object */
  int i;
  int rc = SQLITE_OK;

  pNew = (unicode_tokenizer *) sqlite3_malloc(sizeof(unicode_tokenizer));
  if( pNew==NULL ) return SQLITE_NOMEM;
  memset(pNew, 0, sizeof(unicode_tokenizer));
  pNew->bRemoveDiacritic = 1;

  for(i=0; rc==SQLITE_OK && i<nArg; i++){
    const char *z = azArg[i];
    int n = (int)strlen(z);

    if( n==19 && memcmp("remove_diacritics=1", z, 19)==0 ){
      pNew->bRemoveDiacritic = 1;
    }
    else if( n==19 && memcmp("remove_diacritics=0", z, 19)==0 ){
      pNew->bRemoveDiacritic = 0;
    }
    else if( n>=11 && memcmp("tokenchars=", z, 11)==0 ){
      rc = unicodeAddExceptions(pNew, 1, &z[11], n-11);
    }
    else if( n>=11 && memcmp("separators=", z, 11)==0 ){
      rc = unicodeAddExceptions(pNew, 0, &z[11], n-11);
    }
    else{
      /* Unrecognized argument */
      rc  = SQLITE_ERROR;
    }
  }

  if( rc!=SQLITE_OK ){
    unicodeDestroy((sqlite3_tokenizer *)pNew);
    pNew = 0;
  }
  *pp = (sqlite3_tokenizer *)pNew;
  return rc;
}

/*
** Prepare to begin tokenizing a particular string.  The input
** string to be tokenized is pInput[0..nBytes-1].  A cursor
** used to incrementally tokenize this string is returned in 
** *ppCursor.
*/
static int unicodeOpen(
  sqlite3_tokenizer *p,           /* The tokenizer */
  const char *aInput,             /* Input string */
  int nInput,                     /* Size of string aInput in bytes */
  sqlite3_tokenizer_cursor **pp   /* OUT: New cursor object */
){
  unicode_cursor *pCsr;

  pCsr = (unicode_cursor *)sqlite3_malloc(sizeof(unicode_cursor));
  if( pCsr==0 ){
    return SQLITE_NOMEM;
  }
  memset(pCsr, 0, sizeof(unicode_cursor));

  pCsr->aInput = (const unsigned char *)aInput;
  if( aInput==0 ){
    pCsr->nInput = 0;
  }else if( nInput<0 ){
    pCsr->nInput = (int)strlen(aInput);
  }else{
    pCsr->nInput = nInput;
  }

  *pp = &pCsr->base;
  UNUSED_PARAMETER(p);
  return SQLITE_OK;
}

/*
** Close a tokenization cursor previously opened by a call to
** simpleOpen() above.
*/
static int unicodeClose(sqlite3_tokenizer_cursor *pCursor){
  unicode_cursor *pCsr = (unicode_cursor *) pCursor;
  sqlite3_free(pCsr->zToken);
  sqlite3_free(pCsr);
  return SQLITE_OK;
}

/*
** Extract the next token from a tokenization cursor.  The cursor must
** have been opened by a prior call to simpleOpen().
*/
static int unicodeNext(
  sqlite3_tokenizer_cursor *pC,   /* Cursor returned by simpleOpen */
  const char **paToken,           /* OUT: Token text */
  int *pnToken,                   /* OUT: Number of bytes at *paToken */
  int *piStart,                   /* OUT: Starting offset of token */
  int *piEnd,                     /* OUT: Ending offset of token */
  int *piPos                      /* OUT: Position integer of token */
){
  unicode_cursor *pCsr = (unicode_cursor *)pC;
  unicode_tokenizer *p = ((unicode_tokenizer *)pCsr->base.pTokenizer);
  int iCode = 0;
  char *zOut;
  const unsigned char *z = &pCsr->aInput[pCsr->iOff];
  const unsigned char *zStart = z;
  const unsigned char *zEnd;
  const unsigned char *zTerm = &pCsr->aInput[pCsr->nInput];

  /* Scan past any delimiter characters before the start of the next token.
  ** Return SQLITE_DONE early if this takes us all the way to the end of 
  ** the input.  */
  while( z<zTerm ){
    READ_UTF8(z, zTerm, iCode);
    if( unicodeIsAlnum(p, iCode) ) break;
    zStart = z;
  }
  if( zStart>=zTerm ) return SQLITE_DONE;

  zOut = pCsr->zToken;
  do {
    int iOut;

    /* Grow the output buffer if required. */
    if( (zOut-pCsr->zToken)>=(pCsr->nAlloc-4) ){
      char *zNew = sqlite3_realloc(pCsr->zToken, pCsr->nAlloc+64);
      if( !zNew ) return SQLITE_NOMEM;
      zOut = &zNew[zOut - pCsr->zToken];
      pCsr->zToken = zNew;
      pCsr->nAlloc += 64;
    }

    /* Write the folded case of the last character read to the output */
    zEnd = z;
    iOut = sqlite3FtsUnicodeFold(iCode, p->bRemoveDiacritic);
    if( iOut ){
      WRITE_UTF8(zOut, iOut);
    }

    /* If the cursor is not at EOF, read the next character */
    if( z>=zTerm ) break;
    READ_UTF8(z, zTerm, iCode);
  }while( unicodeIsAlnum(p, iCode) 
       || sqlite3FtsUnicodeIsdiacritic(iCode)
  );

  /* Set the output variables and return. */
  pCsr->iOff = (int)(z - pCsr->aInput);
  *paToken = pCsr->zToken;
  *pnToken = (int)(zOut - pCsr->zToken);
  *piStart = (int)(zStart - pCsr->aInput);
  *piEnd = (int)(zEnd - pCsr->aInput);
  *piPos = pCsr->iToken++;
  return SQLITE_OK;
}

/*
** Set *ppModule to a pointer to the sqlite3_tokenizer_module 
** structure for the unicode tokenizer.
*/
SQLITE_PRIVATE void sqlite3Fts3UnicodeTokenizer(sqlite3_tokenizer_module const **ppModule){
  static const sqlite3_tokenizer_module module = {
    0,
    unicodeCreate,
    unicodeDestroy,
    unicodeOpen,
    unicodeClose,
    unicodeNext,
    0,
  };
  *ppModule = &module;
}

#endif /* !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_FTS3) */
#endif /* ifndef SQLITE_DISABLE_FTS3_UNICODE */

/************** End of fts3_unicode.c ****************************************/
/************** Begin file fts3_unicode2.c ***********************************/
/*
** 2012 May 25
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
*/

/*
** DO NOT EDIT THIS MACHINE GENERATED FILE.
*/

#ifndef SQLITE_DISABLE_FTS3_UNICODE
#if defined(SQLITE_ENABLE_FTS3) || defined(SQLITE_ENABLE_FTS4)

/* #include <assert.h> */

/*
** Return true if the argument corresponds to a unicode codepoint
** classified as either a letter or a number. Otherwise false.
**
** The results are undefined if the value passed to this function
** is less than zero.
*/
SQLITE_PRIVATE int sqlite3FtsUnicodeIsalnum(int c){
  /* Each unsigned integer in the following array corresponds to a contiguous
  ** range of unicode codepoints that are not either letters or numbers (i.e.
  ** codepoints for which this function should return 0).
  **
  ** The most significant 22 bits in each 32-bit value contain the first 
  ** codepoint in the range. The least significant 10 bits are used to store
  ** the size of the range (always at least 1). In other words, the value 
  ** ((C<<22) + N) represents a range of N codepoints starting with codepoint 
  ** C. It is not possible to represent a range larger than 1023 codepoints 
  ** using this format.
  */
  static const unsigned int aEntry[] = {
    0x00000030, 0x0000E807, 0x00016C06, 0x0001EC2F, 0x0002AC07,
    0x0002D001, 0x0002D803, 0x0002EC01, 0x0002FC01, 0x00035C01,
    0x0003DC01, 0x000B0804, 0x000B480E, 0x000B9407, 0x000BB401,
    0x000BBC81, 0x000DD401, 0x000DF801, 0x000E1002, 0x000E1C01,
    0x000FD801, 0x00120808, 0x00156806, 0x00162402, 0x00163C01,
    0x00164437, 0x0017CC02, 0x00180005, 0x00181816, 0x00187802,
    0x00192C15, 0x0019A804, 0x0019C001, 0x001B5001, 0x001B580F,
    0x001B9C07, 0x001BF402, 0x001C000E, 0x001C3C01, 0x001C4401,
    0x001CC01B, 0x001E980B, 0x001FAC09, 0x001FD804, 0x00205804,
    0x00206C09, 0x00209403, 0x0020A405, 0x0020C00F, 0x00216403,
    0x00217801, 0x0023901B, 0x00240004, 0x0024E803, 0x0024F812,
    0x00254407, 0x00258804, 0x0025C001, 0x00260403, 0x0026F001,
    0x0026F807, 0x00271C02, 0x00272C03, 0x00275C01, 0x00278802,
    0x0027C802, 0x0027E802, 0x00280403, 0x0028F001, 0x0028F805,
    0x00291C02, 0x00292C03, 0x00294401, 0x0029C002, 0x0029D401,
    0x002A0403, 0x002AF001, 0x002AF808, 0x002B1C03, 0x002B2C03,
    0x002B8802, 0x002BC002, 0x002C0403, 0x002CF001, 0x002CF807,
    0x002D1C02, 0x002D2C03, 0x002D5802, 0x002D8802, 0x002DC001,
    0x002E0801, 0x002EF805, 0x002F1803, 0x002F2804, 0x002F5C01,
    0x002FCC08, 0x00300403, 0x0030F807, 0x00311803, 0x00312804,
    0x00315402, 0x00318802, 0x0031FC01, 0x00320802, 0x0032F001,
    0x0032F807, 0x00331803, 0x00332804, 0x00335402, 0x00338802,
    0x00340802, 0x0034F807, 0x00351803, 0x00352804, 0x00355C01,
    0x00358802, 0x0035E401, 0x00360802, 0x00372801, 0x00373C06,
    0x00375801, 0x00376008, 0x0037C803, 0x0038C401, 0x0038D007,
    0x0038FC01, 0x00391C09, 0x00396802, 0x003AC401, 0x003AD006,
    0x003AEC02, 0x003B2006, 0x003C041F, 0x003CD00C, 0x003DC417,
    0x003E340B, 0x003E6424, 0x003EF80F, 0x003F380D, 0x0040AC14,
    0x00412806, 0x00415804, 0x00417803, 0x00418803, 0x00419C07,
    0x0041C404, 0x0042080C, 0x00423C01, 0x00426806, 0x0043EC01,
    0x004D740C, 0x004E400A, 0x00500001, 0x0059B402, 0x005A0001,
    0x005A6C02, 0x005BAC03, 0x005C4803, 0x005CC805, 0x005D4802,
    0x005DC802, 0x005ED023, 0x005F6004, 0x005F7401, 0x0060000F,
    0x0062A401, 0x0064800C, 0x0064C00C, 0x00650001, 0x00651002,
    0x0066C011, 0x00672002, 0x00677822, 0x00685C05, 0x00687802,
    0x0069540A, 0x0069801D, 0x0069FC01, 0x006A8007, 0x006AA006,
    0x006C0005, 0x006CD011, 0x006D6823, 0x006E0003, 0x006E840D,
    0x006F980E, 0x006FF004, 0x00709014, 0x0070EC05, 0x0071F802,
    0x00730008, 0x00734019, 0x0073B401, 0x0073C803, 0x00770027,
    0x0077F004, 0x007EF401, 0x007EFC03, 0x007F3403, 0x007F7403,
    0x007FB403, 0x007FF402, 0x00800065, 0x0081A806, 0x0081E805,
    0x00822805, 0x0082801A, 0x00834021, 0x00840002, 0x00840C04,
    0x00842002, 0x00845001, 0x00845803, 0x00847806, 0x00849401,
    0x00849C01, 0x0084A401, 0x0084B801, 0x0084E802, 0x00850005,
    0x00852804, 0x00853C01, 0x00864264, 0x00900027, 0x0091000B,
    0x0092704E, 0x00940200, 0x009C0475, 0x009E53B9, 0x00AD400A,
    0x00B39406, 0x00B3BC03, 0x00B3E404, 0x00B3F802, 0x00B5C001,
    0x00B5FC01, 0x00B7804F, 0x00B8C00C, 0x00BA001A, 0x00BA6C59,
    0x00BC00D6, 0x00BFC00C, 0x00C00005, 0x00C02019, 0x00C0A807,
    0x00C0D802, 0x00C0F403, 0x00C26404, 0x00C28001, 0x00C3EC01,
    0x00C64002, 0x00C6580A, 0x00C70024, 0x00C8001F, 0x00C8A81E,
    0x00C94001, 0x00C98020, 0x00CA2827, 0x00CB003F, 0x00CC0100,
    0x01370040, 0x02924037, 0x0293F802, 0x02983403, 0x0299BC10,
    0x029A7C01, 0x029BC008, 0x029C0017, 0x029C8002, 0x029E2402,
    0x02A00801, 0x02A01801, 0x02A02C01, 0x02A08C09, 0x02A0D804,
    0x02A1D004, 0x02A20002, 0x02A2D011, 0x02A33802, 0x02A38012,
    0x02A3E003, 0x02A4980A, 0x02A51C0D, 0x02A57C01, 0x02A60004,
    0x02A6CC1B, 0x02A77802, 0x02A8A40E, 0x02A90C01, 0x02A93002,
    0x02A97004, 0x02A9DC03, 0x02A9EC01, 0x02AAC001, 0x02AAC803,
    0x02AADC02, 0x02AAF802, 0x02AB0401, 0x02AB7802, 0x02ABAC07,
    0x02ABD402, 0x02AF8C0B, 0x03600001, 0x036DFC02, 0x036FFC02,
    0x037FFC01, 0x03EC7801, 0x03ECA401, 0x03EEC810, 0x03F4F802,
    0x03F7F002, 0x03F8001A, 0x03F88007, 0x03F8C023, 0x03F95013,
    0x03F9A004, 0x03FBFC01, 0x03FC040F, 0x03FC6807, 0x03FCEC06,
    0x03FD6C0B, 0x03FF8007, 0x03FFA007, 0x03FFE405, 0x04040003,
    0x0404DC09, 0x0405E411, 0x0406400C, 0x0407402E, 0x040E7C01,
    0x040F4001, 0x04215C01, 0x04247C01, 0x0424FC01, 0x04280403,
    0x04281402, 0x04283004, 0x0428E003, 0x0428FC01, 0x04294009,
    0x0429FC01, 0x042CE407, 0x04400003, 0x0440E016, 0x04420003,
    0x0442C012, 0x04440003, 0x04449C0E, 0x04450004, 0x04460003,
    0x0446CC0E, 0x04471404, 0x045AAC0D, 0x0491C004, 0x05BD442E,
    0x05BE3C04, 0x074000F6, 0x07440027, 0x0744A4B5, 0x07480046,
    0x074C0057, 0x075B0401, 0x075B6C01, 0x075BEC01, 0x075C5401,
    0x075CD401, 0x075D3C01, 0x075DBC01, 0x075E2401, 0x075EA401,
    0x075F0C01, 0x07BBC002, 0x07C0002C, 0x07C0C064, 0x07C2800F,
    0x07C2C40E, 0x07C3040F, 0x07C3440F, 0x07C4401F, 0x07C4C03C,
    0x07C5C02B, 0x07C7981D, 0x07C8402B, 0x07C90009, 0x07C94002,
    0x07CC0021, 0x07CCC006, 0x07CCDC46, 0x07CE0014, 0x07CE8025,
    0x07CF1805, 0x07CF8011, 0x07D0003F, 0x07D10001, 0x07D108B6,
    0x07D3E404, 0x07D4003E, 0x07D50004, 0x07D54018, 0x07D7EC46,
    0x07D9140B, 0x07DA0046, 0x07DC0074, 0x38000401, 0x38008060,
    0x380400F0,
  };
  static const unsigned int aAscii[4] = {
    0xFFFFFFFF, 0xFC00FFFF, 0xF8000001, 0xF8000001,
  };

  if( c<128 ){
    return ( (aAscii[c >> 5] & (1 << (c & 0x001F)))==0 );
  }else if( c<(1<<22) ){
    unsigned int key = (((unsigned int)c)<<10) | 0x000003FF;
    int iRes = 0;
    int iHi = sizeof(aEntry)/sizeof(aEntry[0]) - 1;
    int iLo = 0;
    while( iHi>=iLo ){
      int iTest = (iHi + iLo) / 2;
      if( key >= aEntry[iTest] ){
        iRes = iTest;
        iLo = iTest+1;
      }else{
        iHi = iTest-1;
      }
    }
    assert( aEntry[0]<key );
    assert( key>=aEntry[iRes] );
    return (((unsigned int)c) >= ((aEntry[iRes]>>10) + (aEntry[iRes]&0x3FF)));
  }
  return 1;
}


/*
** If the argument is a codepoint corresponding to a lowercase letter
** in the ASCII range with a diacritic added, return the codepoint
** of the ASCII letter only. For example, if passed 235 - "LATIN
** SMALL LETTER E WITH DIAERESIS" - return 65 ("LATIN SMALL LETTER
** E"). The resuls of passing a codepoint that corresponds to an
** uppercase letter are undefined.
*/
static int remove_diacritic(int c){
  unsigned short aDia[] = {
        0,  1797,  1848,  1859,  1891,  1928,  1940,  1995, 
     2024,  2040,  2060,  2110,  2168,  2206,  2264,  2286, 
     2344,  2383,  2472,  2488,  2516,  2596,  2668,  2732, 
     2782,  2842,  2894,  2954,  2984,  3000,  3028,  3336, 
     3456,  3696,  3712,  3728,  3744,  3896,  3912,  3928, 
     3968,  4008,  4040,  4106,  4138,  4170,  4202,  4234, 
     4266,  4296,  4312,  4344,  4408,  4424,  4472,  4504, 
     6148,  6198,  6264,  6280,  6360,  6429,  6505,  6529, 
    61448, 61468, 61534, 61592, 61642, 61688, 61704, 61726, 
    61784, 61800, 61836, 61880, 61914, 61948, 61998, 62122, 
    62154, 62200, 62218, 62302, 62364, 62442, 62478, 62536, 
    62554, 62584, 62604, 62640, 62648, 62656, 62664, 62730, 
    62924, 63050, 63082, 63274, 63390, 
  };
  char aChar[] = {
    '\0', 'a',  'c',  'e',  'i',  'n',  'o',  'u',  'y',  'y',  'a',  'c',  
    'd',  'e',  'e',  'g',  'h',  'i',  'j',  'k',  'l',  'n',  'o',  'r',  
    's',  't',  'u',  'u',  'w',  'y',  'z',  'o',  'u',  'a',  'i',  'o',  
    'u',  'g',  'k',  'o',  'j',  'g',  'n',  'a',  'e',  'i',  'o',  'r',  
    'u',  's',  't',  'h',  'a',  'e',  'o',  'y',  '\0', '\0', '\0', '\0', 
    '\0', '\0', '\0', '\0', 'a',  'b',  'd',  'd',  'e',  'f',  'g',  'h',  
    'h',  'i',  'k',  'l',  'l',  'm',  'n',  'p',  'r',  'r',  's',  't',  
    'u',  'v',  'w',  'w',  'x',  'y',  'z',  'h',  't',  'w',  'y',  'a',  
    'e',  'i',  'o',  'u',  'y',  
  };

  unsigned int key = (((unsigned int)c)<<3) | 0x00000007;
  int iRes = 0;
  int iHi = sizeof(aDia)/sizeof(aDia[0]) - 1;
  int iLo = 0;
  while( iHi>=iLo ){
    int iTest = (iHi + iLo) / 2;
    if( key >= aDia[iTest] ){
      iRes = iTest;
      iLo = iTest+1;
    }else{
      iHi = iTest-1;
    }
  }
  assert( key>=aDia[iRes] );
  return ((c > (aDia[iRes]>>3) + (aDia[iRes]&0x07)) ? c : (int)aChar[iRes]);
}


/*
** Return true if the argument interpreted as a unicode codepoint
** is a diacritical modifier character.
*/
SQLITE_PRIVATE int sqlite3FtsUnicodeIsdiacritic(int c){
  unsigned int mask0 = 0x08029FDF;
  unsigned int mask1 = 0x000361F8;
  if( c<768 || c>817 ) return 0;
  return (c < 768+32) ?
      (mask0 & (1 << (c-768))) :
      (mask1 & (1 << (c-768-32)));
}


/*
** Interpret the argument as a unicode codepoint. If the codepoint
** is an upper case character that has a lower case equivalent,
** return the codepoint corresponding to the lower case version.
** Otherwise, return a copy of the argument.
**
** The results are undefined if the value passed to this function
** is less than zero.
*/
SQLITE_PRIVATE int sqlite3FtsUnicodeFold(int c, int bRemoveDiacritic){
  /* Each entry in the following array defines a rule for folding a range
  ** of codepoints to lower case. The rule applies to a range of nRange
  ** codepoints starting at codepoint iCode.
  **
  ** If the least significant bit in flags is clear, then the rule applies
  ** to all nRange codepoints (i.e. all nRange codepoints are upper case and
  ** need to be folded). Or, if it is set, then the rule only applies to
  ** every second codepoint in the range, starting with codepoint C.
  **
  ** The 7 most significant bits in flags are an index into the aiOff[]
  ** array. If a specific codepoint C does require folding, then its lower
  ** case equivalent is ((C + aiOff[flags>>1]) & 0xFFFF).
  **
  ** The contents of this array are generated by parsing the CaseFolding.txt
  ** file distributed as part of the "Unicode Character Database". See
  ** http://www.unicode.org for details.
  */
  static const struct TableEntry {
    unsigned short iCode;
    unsigned char flags;
    unsigned char nRange;
  } aEntry[] = {
    {65, 14, 26},          {181, 64, 1},          {192, 14, 23},
    {216, 14, 7},          {256, 1, 48},          {306, 1, 6},
    {313, 1, 16},          {330, 1, 46},          {376, 116, 1},
    {377, 1, 6},           {383, 104, 1},         {385, 50, 1},
    {386, 1, 4},           {390, 44, 1},          {391, 0, 1},
    {393, 42, 2},          {395, 0, 1},           {398, 32, 1},
    {399, 38, 1},          {400, 40, 1},          {401, 0, 1},
    {403, 42, 1},          {404, 46, 1},          {406, 52, 1},
    {407, 48, 1},          {408, 0, 1},           {412, 52, 1},
    {413, 54, 1},          {415, 56, 1},          {416, 1, 6},
    {422, 60, 1},          {423, 0, 1},           {425, 60, 1},
    {428, 0, 1},           {430, 60, 1},          {431, 0, 1},
    {433, 58, 2},          {435, 1, 4},           {439, 62, 1},
    {440, 0, 1},           {444, 0, 1},           {452, 2, 1},
    {453, 0, 1},           {455, 2, 1},           {456, 0, 1},
    {458, 2, 1},           {459, 1, 18},          {478, 1, 18},
    {497, 2, 1},           {498, 1, 4},           {502, 122, 1},
    {503, 134, 1},         {504, 1, 40},          {544, 110, 1},
    {546, 1, 18},          {570, 70, 1},          {571, 0, 1},
    {573, 108, 1},         {574, 68, 1},          {577, 0, 1},
    {579, 106, 1},         {580, 28, 1},          {581, 30, 1},
    {582, 1, 10},          {837, 36, 1},          {880, 1, 4},
    {886, 0, 1},           {902, 18, 1},          {904, 16, 3},
    {908, 26, 1},          {910, 24, 2},          {913, 14, 17},
    {931, 14, 9},          {962, 0, 1},           {975, 4, 1},
    {976, 140, 1},         {977, 142, 1},         {981, 146, 1},
    {982, 144, 1},         {984, 1, 24},          {1008, 136, 1},
    {1009, 138, 1},        {1012, 130, 1},        {1013, 128, 1},
    {1015, 0, 1},          {1017, 152, 1},        {1018, 0, 1},
    {1021, 110, 3},        {1024, 34, 16},        {1040, 14, 32},
    {1120, 1, 34},         {1162, 1, 54},         {1216, 6, 1},
    {1217, 1, 14},         {1232, 1, 88},         {1329, 22, 38},
    {4256, 66, 38},        {4295, 66, 1},         {4301, 66, 1},
    {7680, 1, 150},        {7835, 132, 1},        {7838, 96, 1},
    {7840, 1, 96},         {7944, 150, 8},        {7960, 150, 6},
    {7976, 150, 8},        {7992, 150, 8},        {8008, 150, 6},
    {8025, 151, 8},        {8040, 150, 8},        {8072, 150, 8},
    {8088, 150, 8},        {8104, 150, 8},        {8120, 150, 2},
    {8122, 126, 2},        {8124, 148, 1},        {8126, 100, 1},
    {8136, 124, 4},        {8140, 148, 1},        {8152, 150, 2},
    {8154, 120, 2},        {8168, 150, 2},        {8170, 118, 2},
    {8172, 152, 1},        {8184, 112, 2},        {8186, 114, 2},
    {8188, 148, 1},        {8486, 98, 1},         {8490, 92, 1},
    {8491, 94, 1},         {8498, 12, 1},         {8544, 8, 16},
    {8579, 0, 1},          {9398, 10, 26},        {11264, 22, 47},
    {11360, 0, 1},         {11362, 88, 1},        {11363, 102, 1},
    {11364, 90, 1},        {11367, 1, 6},         {11373, 84, 1},
    {11374, 86, 1},        {11375, 80, 1},        {11376, 82, 1},
    {11378, 0, 1},         {11381, 0, 1},         {11390, 78, 2},
    {11392, 1, 100},       {11499, 1, 4},         {11506, 0, 1},
    {42560, 1, 46},        {42624, 1, 24},        {42786, 1, 14},
    {42802, 1, 62},        {42873, 1, 4},         {42877, 76, 1},
    {42878, 1, 10},        {42891, 0, 1},         {42893, 74, 1},
    {42896, 1, 4},         {42912, 1, 10},        {42922, 72, 1},
    {65313, 14, 26},       
  };
  static const unsigned short aiOff[] = {
   1,     2,     8,     15,    16,    26,    28,    32,    
   37,    38,    40,    48,    63,    64,    69,    71,    
   79,    80,    116,   202,   203,   205,   206,   207,   
   209,   210,   211,   213,   214,   217,   218,   219,   
   775,   7264,  10792, 10795, 23228, 23256, 30204, 54721, 
   54753, 54754, 54756, 54787, 54793, 54809, 57153, 57274, 
   57921, 58019, 58363, 61722, 65268, 65341, 65373, 65406, 
   65408, 65410, 65415, 65424, 65436, 65439, 65450, 65462, 
   65472, 65476, 65478, 65480, 65482, 65488, 65506, 65511, 
   65514, 65521, 65527, 65528, 65529, 
  };

  int ret = c;

  assert( c>=0 );
  assert( sizeof(unsigned short)==2 && sizeof(unsigned char)==1 );

  if( c<128 ){
    if( c>='A' && c<='Z' ) ret = c + ('a' - 'A');
  }else if( c<65536 ){
    int iHi = sizeof(aEntry)/sizeof(aEntry[0]) - 1;
    int iLo = 0;
    int iRes = -1;

    while( iHi>=iLo ){
      int iTest = (iHi + iLo) / 2;
      int cmp = (c - aEntry[iTest].iCode);
      if( cmp>=0 ){
        iRes = iTest;
        iLo = iTest+1;
      }else{
        iHi = iTest-1;
      }
    }
    assert( iRes<0 || c>=aEntry[iRes].iCode );

    if( iRes>=0 ){
      const struct TableEntry *p = &aEntry[iRes];
      if( c<(p->iCode + p->nRange) && 0==(0x01 & p->flags & (p->iCode ^ c)) ){
        ret = (c + (aiOff[p->flags>>1])) & 0x0000FFFF;
        assert( ret>0 );
      }
    }

    if( bRemoveDiacritic ) ret = remove_diacritic(ret);
  }
  
  else if( c>=66560 && c<66600 ){
    ret = c + 40;
  }

  return ret;
}
#endif /* defined(SQLITE_ENABLE_FTS3) || defined(SQLITE_ENABLE_FTS4) */
#endif /* !defined(SQLITE_DISABLE_FTS3_UNICODE) */

/************** End of fts3_unicode2.c ***************************************/
/************** Begin file rtree.c *******************************************/
/*
** 2001 September 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains code for implementations of the r-tree and r*-tree
** algorithms packaged as an SQLite virtual table module.
*/

/*
** Database Format of R-Tree Tables
** --------------------------------
**
** The data structure for a single virtual r-tree table is stored in three 
** native SQLite tables declared as follows. In each case, the '%' character
** in the table name is replaced with the user-supplied name of the r-tree
** table.
**
**   CREATE TABLE %_node(nodeno INTEGER PRIMARY KEY, data BLOB)
**   CREATE TABLE %_parent(nodeno INTEGER PRIMARY KEY, parentnode INTEGER)
**   CREATE TABLE %_rowid(rowid INTEGER PRIMARY KEY, nodeno INTEGER)
**
** The data for each node of the r-tree structure is stored in the %_node
** table. For each node that is not the root node of the r-tree, there is
** an entry in the %_parent table associating the node with its parent.
** And for each row of data in the table, there is an entry in the %_rowid
** table that maps from the entries rowid to the id of the node that it
** is stored on.
**
** The root node of an r-tree always exists, even if the r-tree table is
** empty. The nodeno of the root node is always 1. All other nodes in the
** table must be the same size as the root node. The content of each node
** is formatted as follows:
**
**   1. If the node is the root node (node 1), then the first 2 bytes
**      of the node contain the tree depth as a big-endian integer.
**      For non-root nodes, the first 2 bytes are left unused.
**
**   2. The next 2 bytes contain the number of entries currently 
**      stored in the node.
**
**   3. The remainder of the node contains the node entries. Each entry
**      consists of a single 8-byte integer followed by an even number
**      of 4-byte coordinates. For leaf nodes the integer is the rowid
**      of a record. For internal nodes it is the node number of a
**      child page.
*/

#if !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_RTREE)

#ifndef SQLITE_CORE
  SQLITE_EXTENSION_INIT1
#else
#endif

/* #include <string.h> */
/* #include <assert.h> */
/* #include <stdio.h> */

#ifndef SQLITE_AMALGAMATION
#include "sqlite3rtree.h"
typedef sqlite3_int64 i64;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
#endif

/*  The following macro is used to suppress compiler warnings.
*/
#ifndef UNUSED_PARAMETER
# define UNUSED_PARAMETER(x) (void)(x)
#endif

typedef struct Rtree Rtree;
typedef struct RtreeCursor RtreeCursor;
typedef struct RtreeNode RtreeNode;
typedef struct RtreeCell RtreeCell;
typedef struct RtreeConstraint RtreeConstraint;
typedef struct RtreeMatchArg RtreeMatchArg;
typedef struct RtreeGeomCallback RtreeGeomCallback;
typedef union RtreeCoord RtreeCoord;
typedef struct RtreeSearchPoint RtreeSearchPoint;

/* The rtree may have between 1 and RTREE_MAX_DIMENSIONS dimensions. */
#define RTREE_MAX_DIMENSIONS 5

/* Size of hash table Rtree.aHash. This hash table is not expected to
** ever contain very many entries, so a fixed number of buckets is 
** used.
*/
#define HASHSIZE 97

/* The xBestIndex method of this virtual table requires an estimate of
** the number of rows in the virtual table to calculate the costs of
** various strategies. If possible, this estimate is loaded from the
** sqlite_stat1 table (with RTREE_MIN_ROWEST as a hard-coded minimum).
** Otherwise, if no sqlite_stat1 entry is available, use 
** RTREE_DEFAULT_ROWEST.
*/
#define RTREE_DEFAULT_ROWEST 1048576
#define RTREE_MIN_ROWEST         100

/* 
** An rtree virtual-table object.
*/
struct Rtree {
  sqlite3_vtab base;          /* Base class.  Must be first */
  sqlite3 *db;                /* Host database connection */
  int iNodeSize;              /* Size in bytes of each node in the node table */
  u8 nDim;                    /* Number of dimensions */
  u8 eCoordType;              /* RTREE_COORD_REAL32 or RTREE_COORD_INT32 */
  u8 nBytesPerCell;           /* Bytes consumed per cell */
  int iDepth;                 /* Current depth of the r-tree structure */
  char *zDb;                  /* Name of database containing r-tree table */
  char *zName;                /* Name of r-tree table */ 
  int nBusy;                  /* Current number of users of this structure */
  i64 nRowEst;                /* Estimated number of rows in this table */

  /* List of nodes removed during a CondenseTree operation. List is
  ** linked together via the pointer normally used for hash chains -
  ** RtreeNode.pNext. RtreeNode.iNode stores the depth of the sub-tree 
  ** headed by the node (leaf nodes have RtreeNode.iNode==0).
  */
  RtreeNode *pDeleted;
  int iReinsertHeight;        /* Height of sub-trees Reinsert() has run on */

  /* Statements to read/write/delete a record from xxx_node */
  sqlite3_stmt *pReadNode;
  sqlite3_stmt *pWriteNode;
  sqlite3_stmt *pDeleteNode;

  /* Statements to read/write/delete a record from xxx_rowid */
  sqlite3_stmt *pReadRowid;
  sqlite3_stmt *pWriteRowid;
  sqlite3_stmt *pDeleteRowid;

  /* Statements to read/write/delete a record from xxx_parent */
  sqlite3_stmt *pReadParent;
  sqlite3_stmt *pWriteParent;
  sqlite3_stmt *pDeleteParent;

  RtreeNode *aHash[HASHSIZE]; /* Hash table of in-memory nodes. */ 
};

/* Possible values for Rtree.eCoordType: */
#define RTREE_COORD_REAL32 0
#define RTREE_COORD_INT32  1

/*
** If SQLITE_RTREE_INT_ONLY is defined, then this virtual table will
** only deal with integer coordinates.  No floating point operations
** will be done.
*/
#ifdef SQLITE_RTREE_INT_ONLY
  typedef sqlite3_int64 RtreeDValue;       /* High accuracy coordinate */
  typedef int RtreeValue;                  /* Low accuracy coordinate */
# define RTREE_ZERO 0
#else
  typedef double RtreeDValue;              /* High accuracy coordinate */
  typedef float RtreeValue;                /* Low accuracy coordinate */
# define RTREE_ZERO 0.0
#endif

/*
** When doing a search of an r-tree, instances of the following structure
** record intermediate results from the tree walk.
**
** The id is always a node-id.  For iLevel>=1 the id is the node-id of
** the node that the RtreeSearchPoint represents.  When iLevel==0, however,
** the id is of the parent node and the cell that RtreeSearchPoint
** represents is the iCell-th entry in the parent node.
*/
struct RtreeSearchPoint {
  RtreeDValue rScore;    /* The score for this node.  Smallest goes first. */
  sqlite3_int64 id;      /* Node ID */
  u8 iLevel;             /* 0=entries.  1=leaf node.  2+ for higher */
  u8 eWithin;            /* PARTLY_WITHIN or FULLY_WITHIN */
  u8 iCell;              /* Cell index within the node */
};

/*
** The minimum number of cells allowed for a node is a third of the 
** maximum. In Gutman's notation:
**
**     m = M/3
**
** If an R*-tree "Reinsert" operation is required, the same number of
** cells are removed from the overfull node and reinserted into the tree.
*/
#define RTREE_MINCELLS(p) ((((p)->iNodeSize-4)/(p)->nBytesPerCell)/3)
#define RTREE_REINSERT(p) RTREE_MINCELLS(p)
#define RTREE_MAXCELLS 51

/*
** The smallest possible node-size is (512-64)==448 bytes. And the largest
** supported cell size is 48 bytes (8 byte rowid + ten 4 byte coordinates).
** Therefore all non-root nodes must contain at least 3 entries. Since 
** 2^40 is greater than 2^64, an r-tree structure always has a depth of
** 40 or less.
*/
#define RTREE_MAX_DEPTH 40


/*
** Number of entries in the cursor RtreeNode cache.  The first entry is
** used to cache the RtreeNode for RtreeCursor.sPoint.  The remaining
** entries cache the RtreeNode for the first elements of the priority queue.
*/
#define RTREE_CACHE_SZ  5

/* 
** An rtree cursor object.
*/
struct RtreeCursor {
  sqlite3_vtab_cursor base;         /* Base class.  Must be first */
  u8 atEOF;                         /* True if at end of search */
  u8 bPoint;                        /* True if sPoint is valid */
  int iStrategy;                    /* Copy of idxNum search parameter */
  int nConstraint;                  /* Number of entries in aConstraint */
  RtreeConstraint *aConstraint;     /* Search constraints. */
  int nPointAlloc;                  /* Number of slots allocated for aPoint[] */
  int nPoint;                       /* Number of slots used in aPoint[] */
  int mxLevel;                      /* iLevel value for root of the tree */
  RtreeSearchPoint *aPoint;         /* Priority queue for search points */
  RtreeSearchPoint sPoint;          /* Cached next search point */
  RtreeNode *aNode[RTREE_CACHE_SZ]; /* Rtree node cache */
  u32 anQueue[RTREE_MAX_DEPTH+1];   /* Number of queued entries by iLevel */
};

/* Return the Rtree of a RtreeCursor */
#define RTREE_OF_CURSOR(X)   ((Rtree*)((X)->base.pVtab))

/*
** A coordinate can be either a floating point number or a integer.  All
** coordinates within a single R-Tree are always of the same time.
*/
union RtreeCoord {
  RtreeValue f;      /* Floating point value */
  int i;             /* Integer value */
  u32 u;             /* Unsigned for byte-order conversions */
};

/*
** The argument is an RtreeCoord. Return the value stored within the RtreeCoord
** formatted as a RtreeDValue (double or int64). This macro assumes that local
** variable pRtree points to the Rtree structure associated with the
** RtreeCoord.
*/
#ifdef SQLITE_RTREE_INT_ONLY
# define DCOORD(coord) ((RtreeDValue)coord.i)
#else
# define DCOORD(coord) (                           \
    (pRtree->eCoordType==RTREE_COORD_REAL32) ?      \
      ((double)coord.f) :                           \
      ((double)coord.i)                             \
  )
#endif

/*
** A search constraint.
*/
struct RtreeConstraint {
  int iCoord;                     /* Index of constrained coordinate */
  int op;                         /* Constraining operation */
  union {
    RtreeDValue rValue;             /* Constraint value. */
    int (*xGeom)(sqlite3_rtree_geometry*,int,RtreeDValue*,int*);
    int (*xQueryFunc)(sqlite3_rtree_query_info*);
  } u;
  sqlite3_rtree_query_info *pInfo;  /* xGeom and xQueryFunc argument */
};

/* Possible values for RtreeConstraint.op */
#define RTREE_EQ    0x41  /* A */
#define RTREE_LE    0x42  /* B */
#define RTREE_LT    0x43  /* C */
#define RTREE_GE    0x44  /* D */
#define RTREE_GT    0x45  /* E */
#define RTREE_MATCH 0x46  /* F: Old-style sqlite3_rtree_geometry_callback() */
#define RTREE_QUERY 0x47  /* G: New-style sqlite3_rtree_query_callback() */


/* 
** An rtree structure node.
*/
struct RtreeNode {
  RtreeNode *pParent;         /* Parent node */
  i64 iNode;                  /* The node number */
  int nRef;                   /* Number of references to this node */
  int isDirty;                /* True if the node needs to be written to disk */
  u8 *zData;                  /* Content of the node, as should be on disk */
  RtreeNode *pNext;           /* Next node in this hash collision chain */
};

/* Return the number of cells in a node  */
#define NCELL(pNode) readInt16(&(pNode)->zData[2])

/* 
** A single cell from a node, deserialized
*/
struct RtreeCell {
  i64 iRowid;                                 /* Node or entry ID */
  RtreeCoord aCoord[RTREE_MAX_DIMENSIONS*2];  /* Bounding box coordinates */
};


/*
** This object becomes the sqlite3_user_data() for the SQL functions
** that are created by sqlite3_rtree_geometry_callback() and
** sqlite3_rtree_query_callback() and which appear on the right of MATCH
** operators in order to constrain a search.
**
** xGeom and xQueryFunc are the callback functions.  Exactly one of 
** xGeom and xQueryFunc fields is non-NULL, depending on whether the
** SQL function was created using sqlite3_rtree_geometry_callback() or
** sqlite3_rtree_query_callback().
** 
** This object is deleted automatically by the destructor mechanism in
** sqlite3_create_function_v2().
*/
struct RtreeGeomCallback {
  int (*xGeom)(sqlite3_rtree_geometry*, int, RtreeDValue*, int*);
  int (*xQueryFunc)(sqlite3_rtree_query_info*);
  void (*xDestructor)(void*);
  void *pContext;
};


/*
** Value for the first field of every RtreeMatchArg object. The MATCH
** operator tests that the first field of a blob operand matches this
** value to avoid operating on invalid blobs (which could cause a segfault).
*/
#define RTREE_GEOMETRY_MAGIC 0x891245AB

/*
** An instance of this structure (in the form of a BLOB) is returned by
** the SQL functions that sqlite3_rtree_geometry_callback() and
** sqlite3_rtree_query_callback() create, and is read as the right-hand
** operand to the MATCH operator of an R-Tree.
*/
struct RtreeMatchArg {
  u32 magic;                  /* Always RTREE_GEOMETRY_MAGIC */
  RtreeGeomCallback cb;       /* Info about the callback functions */
  int nParam;                 /* Number of parameters to the SQL function */
  RtreeDValue aParam[1];      /* Values for parameters to the SQL function */
};

#ifndef MAX
# define MAX(x,y) ((x) < (y) ? (y) : (x))
#endif
#ifndef MIN
# define MIN(x,y) ((x) > (y) ? (y) : (x))
#endif

/*
** Functions to deserialize a 16 bit integer, 32 bit real number and
** 64 bit integer. The deserialized value is returned.
*/
static int readInt16(u8 *p){
  return (p[0]<<8) + p[1];
}
static void readCoord(u8 *p, RtreeCoord *pCoord){
  u32 i = (
    (((u32)p[0]) << 24) + 
    (((u32)p[1]) << 16) + 
    (((u32)p[2]) <<  8) + 
    (((u32)p[3]) <<  0)
  );
  *(u32 *)pCoord = i;
}
static i64 readInt64(u8 *p){
  return (
    (((i64)p[0]) << 56) + 
    (((i64)p[1]) << 48) + 
    (((i64)p[2]) << 40) + 
    (((i64)p[3]) << 32) + 
    (((i64)p[4]) << 24) + 
    (((i64)p[5]) << 16) + 
    (((i64)p[6]) <<  8) + 
    (((i64)p[7]) <<  0)
  );
}

/*
** Functions to serialize a 16 bit integer, 32 bit real number and
** 64 bit integer. The value returned is the number of bytes written
** to the argument buffer (always 2, 4 and 8 respectively).
*/
static int writeInt16(u8 *p, int i){
  p[0] = (i>> 8)&0xFF;
  p[1] = (i>> 0)&0xFF;
  return 2;
}
static int writeCoord(u8 *p, RtreeCoord *pCoord){
  u32 i;
  assert( sizeof(RtreeCoord)==4 );
  assert( sizeof(u32)==4 );
  i = *(u32 *)pCoord;
  p[0] = (i>>24)&0xFF;
  p[1] = (i>>16)&0xFF;
  p[2] = (i>> 8)&0xFF;
  p[3] = (i>> 0)&0xFF;
  return 4;
}
static int writeInt64(u8 *p, i64 i){
  p[0] = (i>>56)&0xFF;
  p[1] = (i>>48)&0xFF;
  p[2] = (i>>40)&0xFF;
  p[3] = (i>>32)&0xFF;
  p[4] = (i>>24)&0xFF;
  p[5] = (i>>16)&0xFF;
  p[6] = (i>> 8)&0xFF;
  p[7] = (i>> 0)&0xFF;
  return 8;
}

/*
** Increment the reference count of node p.
*/
static void nodeReference(RtreeNode *p){
  if( p ){
    p->nRef++;
  }
}

/*
** Clear the content of node p (set all bytes to 0x00).
*/
static void nodeZero(Rtree *pRtree, RtreeNode *p){
  memset(&p->zData[2], 0, pRtree->iNodeSize-2);
  p->isDirty = 1;
}

/*
** Given a node number iNode, return the corresponding key to use
** in the Rtree.aHash table.
*/
static int nodeHash(i64 iNode){
  return iNode % HASHSIZE;
}

/*
** Search the node hash table for node iNode. If found, return a pointer
** to it. Otherwise, return 0.
*/
static RtreeNode *nodeHashLookup(Rtree *pRtree, i64 iNode){
  RtreeNode *p;
  for(p=pRtree->aHash[nodeHash(iNode)]; p && p->iNode!=iNode; p=p->pNext);
  return p;
}

/*
** Add node pNode to the node hash table.
*/
static void nodeHashInsert(Rtree *pRtree, RtreeNode *pNode){
  int iHash;
  assert( pNode->pNext==0 );
  iHash = nodeHash(pNode->iNode);
  pNode->pNext = pRtree->aHash[iHash];
  pRtree->aHash[iHash] = pNode;
}

/*
** Remove node pNode from the node hash table.
*/
static void nodeHashDelete(Rtree *pRtree, RtreeNode *pNode){
  RtreeNode **pp;
  if( pNode->iNode!=0 ){
    pp = &pRtree->aHash[nodeHash(pNode->iNode)];
    for( ; (*pp)!=pNode; pp = &(*pp)->pNext){ assert(*pp); }
    *pp = pNode->pNext;
    pNode->pNext = 0;
  }
}

/*
** Allocate and return new r-tree node. Initially, (RtreeNode.iNode==0),
** indicating that node has not yet been assigned a node number. It is
** assigned a node number when nodeWrite() is called to write the
** node contents out to the database.
*/
static RtreeNode *nodeNew(Rtree *pRtree, RtreeNode *pParent){
  RtreeNode *pNode;
  pNode = (RtreeNode *)sqlite3_malloc(sizeof(RtreeNode) + pRtree->iNodeSize);
  if( pNode ){
    memset(pNode, 0, sizeof(RtreeNode) + pRtree->iNodeSize);
    pNode->zData = (u8 *)&pNode[1];
    pNode->nRef = 1;
    pNode->pParent = pParent;
    pNode->isDirty = 1;
    nodeReference(pParent);
  }
  return pNode;
}

/*
** Obtain a reference to an r-tree node.
*/
static int nodeAcquire(
  Rtree *pRtree,             /* R-tree structure */
  i64 iNode,                 /* Node number to load */
  RtreeNode *pParent,        /* Either the parent node or NULL */
  RtreeNode **ppNode         /* OUT: Acquired node */
){
  int rc;
  int rc2 = SQLITE_OK;
  RtreeNode *pNode;

  /* Check if the requested node is already in the hash table. If so,
  ** increase its reference count and return it.
  */
  if( (pNode = nodeHashLookup(pRtree, iNode)) ){
    assert( !pParent || !pNode->pParent || pNode->pParent==pParent );
    if( pParent && !pNode->pParent ){
      nodeReference(pParent);
      pNode->pParent = pParent;
    }
    pNode->nRef++;
    *ppNode = pNode;
    return SQLITE_OK;
  }

  sqlite3_bind_int64(pRtree->pReadNode, 1, iNode);
  rc = sqlite3_step(pRtree->pReadNode);
  if( rc==SQLITE_ROW ){
    const u8 *zBlob = sqlite3_column_blob(pRtree->pReadNode, 0);
    if( pRtree->iNodeSize==sqlite3_column_bytes(pRtree->pReadNode, 0) ){
      pNode = (RtreeNode *)sqlite3_malloc(sizeof(RtreeNode)+pRtree->iNodeSize);
      if( !pNode ){
        rc2 = SQLITE_NOMEM;
      }else{
        pNode->pParent = pParent;
        pNode->zData = (u8 *)&pNode[1];
        pNode->nRef = 1;
        pNode->iNode = iNode;
        pNode->isDirty = 0;
        pNode->pNext = 0;
        memcpy(pNode->zData, zBlob, pRtree->iNodeSize);
        nodeReference(pParent);
      }
    }
  }
  rc = sqlite3_reset(pRtree->pReadNode);
  if( rc==SQLITE_OK ) rc = rc2;

  /* If the root node was just loaded, set pRtree->iDepth to the height
  ** of the r-tree structure. A height of zero means all data is stored on
  ** the root node. A height of one means the children of the root node
  ** are the leaves, and so on. If the depth as specified on the root node
  ** is greater than RTREE_MAX_DEPTH, the r-tree structure must be corrupt.
  */
  if( pNode && iNode==1 ){
    pRtree->iDepth = readInt16(pNode->zData);
    if( pRtree->iDepth>RTREE_MAX_DEPTH ){
      rc = SQLITE_CORRUPT_VTAB;
    }
  }

  /* If no error has occurred so far, check if the "number of entries"
  ** field on the node is too large. If so, set the return code to 
  ** SQLITE_CORRUPT_VTAB.
  */
  if( pNode && rc==SQLITE_OK ){
    if( NCELL(pNode)>((pRtree->iNodeSize-4)/pRtree->nBytesPerCell) ){
      rc = SQLITE_CORRUPT_VTAB;
    }
  }

  if( rc==SQLITE_OK ){
    if( pNode!=0 ){
      nodeHashInsert(pRtree, pNode);
    }else{
      rc = SQLITE_CORRUPT_VTAB;
    }
    *ppNode = pNode;
  }else{
    sqlite3_free(pNode);
    *ppNode = 0;
  }

  return rc;
}

/*
** Overwrite cell iCell of node pNode with the contents of pCell.
*/
static void nodeOverwriteCell(
  Rtree *pRtree,             /* The overall R-Tree */
  RtreeNode *pNode,          /* The node into which the cell is to be written */
  RtreeCell *pCell,          /* The cell to write */
  int iCell                  /* Index into pNode into which pCell is written */
){
  int ii;
  u8 *p = &pNode->zData[4 + pRtree->nBytesPerCell*iCell];
  p += writeInt64(p, pCell->iRowid);
  for(ii=0; ii<(pRtree->nDim*2); ii++){
    p += writeCoord(p, &pCell->aCoord[ii]);
  }
  pNode->isDirty = 1;
}

/*
** Remove the cell with index iCell from node pNode.
*/
static void nodeDeleteCell(Rtree *pRtree, RtreeNode *pNode, int iCell){
  u8 *pDst = &pNode->zData[4 + pRtree->nBytesPerCell*iCell];
  u8 *pSrc = &pDst[pRtree->nBytesPerCell];
  int nByte = (NCELL(pNode) - iCell - 1) * pRtree->nBytesPerCell;
  memmove(pDst, pSrc, nByte);
  writeInt16(&pNode->zData[2], NCELL(pNode)-1);
  pNode->isDirty = 1;
}

/*
** Insert the contents of cell pCell into node pNode. If the insert
** is successful, return SQLITE_OK.
**
** If there is not enough free space in pNode, return SQLITE_FULL.
*/
static int nodeInsertCell(
  Rtree *pRtree,                /* The overall R-Tree */
  RtreeNode *pNode,             /* Write new cell into this node */
  RtreeCell *pCell              /* The cell to be inserted */
){
  int nCell;                    /* Current number of cells in pNode */
  int nMaxCell;                 /* Maximum number of cells for pNode */

  nMaxCell = (pRtree->iNodeSize-4)/pRtree->nBytesPerCell;
  nCell = NCELL(pNode);

  assert( nCell<=nMaxCell );
  if( nCell<nMaxCell ){
    nodeOverwriteCell(pRtree, pNode, pCell, nCell);
    writeInt16(&pNode->zData[2], nCell+1);
    pNode->isDirty = 1;
  }

  return (nCell==nMaxCell);
}

/*
** If the node is dirty, write it out to the database.
*/
static int nodeWrite(Rtree *pRtree, RtreeNode *pNode){
  int rc = SQLITE_OK;
  if( pNode->isDirty ){
    sqlite3_stmt *p = pRtree->pWriteNode;
    if( pNode->iNode ){
      sqlite3_bind_int64(p, 1, pNode->iNode);
    }else{
      sqlite3_bind_null(p, 1);
    }
    sqlite3_bind_blob(p, 2, pNode->zData, pRtree->iNodeSize, SQLITE_STATIC);
    sqlite3_step(p);
    pNode->isDirty = 0;
    rc = sqlite3_reset(p);
    if( pNode->iNode==0 && rc==SQLITE_OK ){
      pNode->iNode = sqlite3_last_insert_rowid(pRtree->db);
      nodeHashInsert(pRtree, pNode);
    }
  }
  return rc;
}

/*
** Release a reference to a node. If the node is dirty and the reference
** count drops to zero, the node data is written to the database.
*/
static int nodeRelease(Rtree *pRtree, RtreeNode *pNode){
  int rc = SQLITE_OK;
  if( pNode ){
    assert( pNode->nRef>0 );
    pNode->nRef--;
    if( pNode->nRef==0 ){
      if( pNode->iNode==1 ){
        pRtree->iDepth = -1;
      }
      if( pNode->pParent ){
        rc = nodeRelease(pRtree, pNode->pParent);
      }
      if( rc==SQLITE_OK ){
        rc = nodeWrite(pRtree, pNode);
      }
      nodeHashDelete(pRtree, pNode);
      sqlite3_free(pNode);
    }
  }
  return rc;
}

/*
** Return the 64-bit integer value associated with cell iCell of
** node pNode. If pNode is a leaf node, this is a rowid. If it is
** an internal node, then the 64-bit integer is a child page number.
*/
static i64 nodeGetRowid(
  Rtree *pRtree,       /* The overall R-Tree */
  RtreeNode *pNode,    /* The node from which to extract the ID */
  int iCell            /* The cell index from which to extract the ID */
){
  assert( iCell<NCELL(pNode) );
  return readInt64(&pNode->zData[4 + pRtree->nBytesPerCell*iCell]);
}

/*
** Return coordinate iCoord from cell iCell in node pNode.
*/
static void nodeGetCoord(
  Rtree *pRtree,               /* The overall R-Tree */
  RtreeNode *pNode,            /* The node from which to extract a coordinate */
  int iCell,                   /* The index of the cell within the node */
  int iCoord,                  /* Which coordinate to extract */
  RtreeCoord *pCoord           /* OUT: Space to write result to */
){
  readCoord(&pNode->zData[12 + pRtree->nBytesPerCell*iCell + 4*iCoord], pCoord);
}

/*
** Deserialize cell iCell of node pNode. Populate the structure pointed
** to by pCell with the results.
*/
static void nodeGetCell(
  Rtree *pRtree,               /* The overall R-Tree */
  RtreeNode *pNode,            /* The node containing the cell to be read */
  int iCell,                   /* Index of the cell within the node */
  RtreeCell *pCell             /* OUT: Write the cell contents here */
){
  u8 *pData;
  u8 *pEnd;
  RtreeCoord *pCoord;
  pCell->iRowid = nodeGetRowid(pRtree, pNode, iCell);
  pData = pNode->zData + (12 + pRtree->nBytesPerCell*iCell);
  pEnd = pData + pRtree->nDim*8;
  pCoord = pCell->aCoord;
  for(; pData<pEnd; pData+=4, pCoord++){
    readCoord(pData, pCoord);
  }
}


/* Forward declaration for the function that does the work of
** the virtual table module xCreate() and xConnect() methods.
*/
static int rtreeInit(
  sqlite3 *, void *, int, const char *const*, sqlite3_vtab **, char **, int
);

/* 
** Rtree virtual table module xCreate method.
*/
static int rtreeCreate(
  sqlite3 *db,
  void *pAux,
  int argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  return rtreeInit(db, pAux, argc, argv, ppVtab, pzErr, 1);
}

/* 
** Rtree virtual table module xConnect method.
*/
static int rtreeConnect(
  sqlite3 *db,
  void *pAux,
  int argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  return rtreeInit(db, pAux, argc, argv, ppVtab, pzErr, 0);
}

/*
** Increment the r-tree reference count.
*/
static void rtreeReference(Rtree *pRtree){
  pRtree->nBusy++;
}

/*
** Decrement the r-tree reference count. When the reference count reaches
** zero the structure is deleted.
*/
static void rtreeRelease(Rtree *pRtree){
  pRtree->nBusy--;
  if( pRtree->nBusy==0 ){
    sqlite3_finalize(pRtree->pReadNode);
    sqlite3_finalize(pRtree->pWriteNode);
    sqlite3_finalize(pRtree->pDeleteNode);
    sqlite3_finalize(pRtree->pReadRowid);
    sqlite3_finalize(pRtree->pWriteRowid);
    sqlite3_finalize(pRtree->pDeleteRowid);
    sqlite3_finalize(pRtree->pReadParent);
    sqlite3_finalize(pRtree->pWriteParent);
    sqlite3_finalize(pRtree->pDeleteParent);
    sqlite3_free(pRtree);
  }
}

/* 
** Rtree virtual table module xDisconnect method.
*/
static int rtreeDisconnect(sqlite3_vtab *pVtab){
  rtreeRelease((Rtree *)pVtab);
  return SQLITE_OK;
}

/* 
** Rtree virtual table module xDestroy method.
*/
static int rtreeDestroy(sqlite3_vtab *pVtab){
  Rtree *pRtree = (Rtree *)pVtab;
  int rc;
  char *zCreate = sqlite3_mprintf(
    "DROP TABLE '%q'.'%q_node';"
    "DROP TABLE '%q'.'%q_rowid';"
    "DROP TABLE '%q'.'%q_parent';",
    pRtree->zDb, pRtree->zName, 
    pRtree->zDb, pRtree->zName,
    pRtree->zDb, pRtree->zName
  );
  if( !zCreate ){
    rc = SQLITE_NOMEM;
  }else{
    rc = sqlite3_exec(pRtree->db, zCreate, 0, 0, 0);
    sqlite3_free(zCreate);
  }
  if( rc==SQLITE_OK ){
    rtreeRelease(pRtree);
  }

  return rc;
}

/* 
** Rtree virtual table module xOpen method.
*/
static int rtreeOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  int rc = SQLITE_NOMEM;
  RtreeCursor *pCsr;

  pCsr = (RtreeCursor *)sqlite3_malloc(sizeof(RtreeCursor));
  if( pCsr ){
    memset(pCsr, 0, sizeof(RtreeCursor));
    pCsr->base.pVtab = pVTab;
    rc = SQLITE_OK;
  }
  *ppCursor = (sqlite3_vtab_cursor *)pCsr;

  return rc;
}


/*
** Free the RtreeCursor.aConstraint[] array and its contents.
*/
static void freeCursorConstraints(RtreeCursor *pCsr){
  if( pCsr->aConstraint ){
    int i;                        /* Used to iterate through constraint array */
    for(i=0; i<pCsr->nConstraint; i++){
      sqlite3_rtree_query_info *pInfo = pCsr->aConstraint[i].pInfo;
      if( pInfo ){
        if( pInfo->xDelUser ) pInfo->xDelUser(pInfo->pUser);
        sqlite3_free(pInfo);
      }
    }
    sqlite3_free(pCsr->aConstraint);
    pCsr->aConstraint = 0;
  }
}

/* 
** Rtree virtual table module xClose method.
*/
static int rtreeClose(sqlite3_vtab_cursor *cur){
  Rtree *pRtree = (Rtree *)(cur->pVtab);
  int ii;
  RtreeCursor *pCsr = (RtreeCursor *)cur;
  freeCursorConstraints(pCsr);
  sqlite3_free(pCsr->aPoint);
  for(ii=0; ii<RTREE_CACHE_SZ; ii++) nodeRelease(pRtree, pCsr->aNode[ii]);
  sqlite3_free(pCsr);
  return SQLITE_OK;
}

/*
** Rtree virtual table module xEof method.
**
** Return non-zero if the cursor does not currently point to a valid 
** record (i.e if the scan has finished), or zero otherwise.
*/
static int rtreeEof(sqlite3_vtab_cursor *cur){
  RtreeCursor *pCsr = (RtreeCursor *)cur;
  return pCsr->atEOF;
}

/*
** Convert raw bits from the on-disk RTree record into a coordinate value.
** The on-disk format is big-endian and needs to be converted for little-
** endian platforms.  The on-disk record stores integer coordinates if
** eInt is true and it stores 32-bit floating point records if eInt is
** false.  a[] is the four bytes of the on-disk record to be decoded.
** Store the results in "r".
**
** There are three versions of this macro, one each for little-endian and
** big-endian processors and a third generic implementation.  The endian-
** specific implementations are much faster and are preferred if the
** processor endianness is known at compile-time.  The SQLITE_BYTEORDER
** macro is part of sqliteInt.h and hence the endian-specific
** implementation will only be used if this module is compiled as part
** of the amalgamation.
*/
#if defined(SQLITE_BYTEORDER) && SQLITE_BYTEORDER==1234
#define RTREE_DECODE_COORD(eInt, a, r) {                        \
    RtreeCoord c;    /* Coordinate decoded */                   \
    memcpy(&c.u,a,4);                                           \
    c.u = ((c.u>>24)&0xff)|((c.u>>8)&0xff00)|                   \
          ((c.u&0xff)<<24)|((c.u&0xff00)<<8);                   \
    r = eInt ? (sqlite3_rtree_dbl)c.i : (sqlite3_rtree_dbl)c.f; \
}
#elif defined(SQLITE_BYTEORDER) && SQLITE_BYTEORDER==4321
#define RTREE_DECODE_COORD(eInt, a, r) {                        \
    RtreeCoord c;    /* Coordinate decoded */                   \
    memcpy(&c.u,a,4);                                           \
    r = eInt ? (sqlite3_rtree_dbl)c.i : (sqlite3_rtree_dbl)c.f; \
}
#else
#define RTREE_DECODE_COORD(eInt, a, r) {                        \
    RtreeCoord c;    /* Coordinate decoded */                   \
    c.u = ((u32)a[0]<<24) + ((u32)a[1]<<16)                     \
           +((u32)a[2]<<8) + a[3];                              \
    r = eInt ? (sqlite3_rtree_dbl)c.i : (sqlite3_rtree_dbl)c.f; \
}
#endif

/*
** Check the RTree node or entry given by pCellData and p against the MATCH
** constraint pConstraint.  
*/
static int rtreeCallbackConstraint(
  RtreeConstraint *pConstraint,  /* The constraint to test */
  int eInt,                      /* True if RTree holding integer coordinates */
  u8 *pCellData,                 /* Raw cell content */
  RtreeSearchPoint *pSearch,     /* Container of this cell */
  sqlite3_rtree_dbl *prScore,    /* OUT: score for the cell */
  int *peWithin                  /* OUT: visibility of the cell */
){
  int i;                                                /* Loop counter */
  sqlite3_rtree_query_info *pInfo = pConstraint->pInfo; /* Callback info */
  int nCoord = pInfo->nCoord;                           /* No. of coordinates */
  int rc;                                             /* Callback return code */
  sqlite3_rtree_dbl aCoord[RTREE_MAX_DIMENSIONS*2];   /* Decoded coordinates */

  assert( pConstraint->op==RTREE_MATCH || pConstraint->op==RTREE_QUERY );
  assert( nCoord==2 || nCoord==4 || nCoord==6 || nCoord==8 || nCoord==10 );

  if( pConstraint->op==RTREE_QUERY && pSearch->iLevel==1 ){
    pInfo->iRowid = readInt64(pCellData);
  }
  pCellData += 8;
  for(i=0; i<nCoord; i++, pCellData += 4){
    RTREE_DECODE_COORD(eInt, pCellData, aCoord[i]);
  }
  if( pConstraint->op==RTREE_MATCH ){
    rc = pConstraint->u.xGeom((sqlite3_rtree_geometry*)pInfo,
                              nCoord, aCoord, &i);
    if( i==0 ) *peWithin = NOT_WITHIN;
    *prScore = RTREE_ZERO;
  }else{
    pInfo->aCoord = aCoord;
    pInfo->iLevel = pSearch->iLevel - 1;
    pInfo->rScore = pInfo->rParentScore = pSearch->rScore;
    pInfo->eWithin = pInfo->eParentWithin = pSearch->eWithin;
    rc = pConstraint->u.xQueryFunc(pInfo);
    if( pInfo->eWithin<*peWithin ) *peWithin = pInfo->eWithin;
    if( pInfo->rScore<*prScore || *prScore<RTREE_ZERO ){
      *prScore = pInfo->rScore;
    }
  }
  return rc;
}

/* 
** Check the internal RTree node given by pCellData against constraint p.
** If this constraint cannot be satisfied by any child within the node,
** set *peWithin to NOT_WITHIN.
*/
static void rtreeNonleafConstraint(
  RtreeConstraint *p,        /* The constraint to test */
  int eInt,                  /* True if RTree holds integer coordinates */
  u8 *pCellData,             /* Raw cell content as appears on disk */
  int *peWithin              /* Adjust downward, as appropriate */
){
  sqlite3_rtree_dbl val;     /* Coordinate value convert to a double */

  /* p->iCoord might point to either a lower or upper bound coordinate
  ** in a coordinate pair.  But make pCellData point to the lower bound.
  */
  pCellData += 8 + 4*(p->iCoord&0xfe);

  assert(p->op==RTREE_LE || p->op==RTREE_LT || p->op==RTREE_GE 
      || p->op==RTREE_GT || p->op==RTREE_EQ );
  switch( p->op ){
    case RTREE_LE:
    case RTREE_LT:
    case RTREE_EQ:
      RTREE_DECODE_COORD(eInt, pCellData, val);
      /* val now holds the lower bound of the coordinate pair */
      if( p->u.rValue>=val ) return;
      if( p->op!=RTREE_EQ ) break;  /* RTREE_LE and RTREE_LT end here */
      /* Fall through for the RTREE_EQ case */

    default: /* RTREE_GT or RTREE_GE,  or fallthrough of RTREE_EQ */
      pCellData += 4;
      RTREE_DECODE_COORD(eInt, pCellData, val);
      /* val now holds the upper bound of the coordinate pair */
      if( p->u.rValue<=val ) return;
  }
  *peWithin = NOT_WITHIN;
}

/*
** Check the leaf RTree cell given by pCellData against constraint p.
** If this constraint is not satisfied, set *peWithin to NOT_WITHIN.
** If the constraint is satisfied, leave *peWithin unchanged.
**
** The constraint is of the form:  xN op $val
**
** The op is given by p->op.  The xN is p->iCoord-th coordinate in
** pCellData.  $val is given by p->u.rValue.
*/
static void rtreeLeafConstraint(
  RtreeConstraint *p,        /* The constraint to test */
  int eInt,                  /* True if RTree holds integer coordinates */
  u8 *pCellData,             /* Raw cell content as appears on disk */
  int *peWithin              /* Adjust downward, as appropriate */
){
  RtreeDValue xN;      /* Coordinate value converted to a double */

  assert(p->op==RTREE_LE || p->op==RTREE_LT || p->op==RTREE_GE 
      || p->op==RTREE_GT || p->op==RTREE_EQ );
  pCellData += 8 + p->iCoord*4;
  RTREE_DECODE_COORD(eInt, pCellData, xN);
  switch( p->op ){
    case RTREE_LE: if( xN <= p->u.rValue ) return;  break;
    case RTREE_LT: if( xN <  p->u.rValue ) return;  break;
    case RTREE_GE: if( xN >= p->u.rValue ) return;  break;
    case RTREE_GT: if( xN >  p->u.rValue ) return;  break;
    default:       if( xN == p->u.rValue ) return;  break;
  }
  *peWithin = NOT_WITHIN;
}

/*
** One of the cells in node pNode is guaranteed to have a 64-bit 
** integer value equal to iRowid. Return the index of this cell.
*/
static int nodeRowidIndex(
  Rtree *pRtree, 
  RtreeNode *pNode, 
  i64 iRowid,
  int *piIndex
){
  int ii;
  int nCell = NCELL(pNode);
  assert( nCell<200 );
  for(ii=0; ii<nCell; ii++){
    if( nodeGetRowid(pRtree, pNode, ii)==iRowid ){
      *piIndex = ii;
      return SQLITE_OK;
    }
  }
  return SQLITE_CORRUPT_VTAB;
}

/*
** Return the index of the cell containing a pointer to node pNode
** in its parent. If pNode is the root node, return -1.
*/
static int nodeParentIndex(Rtree *pRtree, RtreeNode *pNode, int *piIndex){
  RtreeNode *pParent = pNode->pParent;
  if( pParent ){
    return nodeRowidIndex(pRtree, pParent, pNode->iNode, piIndex);
  }
  *piIndex = -1;
  return SQLITE_OK;
}

/*
** Compare two search points.  Return negative, zero, or positive if the first
** is less than, equal to, or greater than the second.
**
** The rScore is the primary key.  Smaller rScore values come first.
** If the rScore is a tie, then use iLevel as the tie breaker with smaller
** iLevel values coming first.  In this way, if rScore is the same for all
** SearchPoints, then iLevel becomes the deciding factor and the result
** is a depth-first search, which is the desired default behavior.
*/
static int rtreeSearchPointCompare(
  const RtreeSearchPoint *pA,
  const RtreeSearchPoint *pB
){
  if( pA->rScore<pB->rScore ) return -1;
  if( pA->rScore>pB->rScore ) return +1;
  if( pA->iLevel<pB->iLevel ) return -1;
  if( pA->iLevel>pB->iLevel ) return +1;
  return 0;
}

/*
** Interchange to search points in a cursor.
*/
static void rtreeSearchPointSwap(RtreeCursor *p, int i, int j){
  RtreeSearchPoint t = p->aPoint[i];
  assert( i<j );
  p->aPoint[i] = p->aPoint[j];
  p->aPoint[j] = t;
  i++; j++;
  if( i<RTREE_CACHE_SZ ){
    if( j>=RTREE_CACHE_SZ ){
      nodeRelease(RTREE_OF_CURSOR(p), p->aNode[i]);
      p->aNode[i] = 0;
    }else{
      RtreeNode *pTemp = p->aNode[i];
      p->aNode[i] = p->aNode[j];
      p->aNode[j] = pTemp;
    }
  }
}

/*
** Return the search point with the lowest current score.
*/
static RtreeSearchPoint *rtreeSearchPointFirst(RtreeCursor *pCur){
  return pCur->bPoint ? &pCur->sPoint : pCur->nPoint ? pCur->aPoint : 0;
}

/*
** Get the RtreeNode for the search point with the lowest score.
*/
static RtreeNode *rtreeNodeOfFirstSearchPoint(RtreeCursor *pCur, int *pRC){
  sqlite3_int64 id;
  int ii = 1 - pCur->bPoint;
  assert( ii==0 || ii==1 );
  assert( pCur->bPoint || pCur->nPoint );
  if( pCur->aNode[ii]==0 ){
    assert( pRC!=0 );
    id = ii ? pCur->aPoint[0].id : pCur->sPoint.id;
    *pRC = nodeAcquire(RTREE_OF_CURSOR(pCur), id, 0, &pCur->aNode[ii]);
  }
  return pCur->aNode[ii];
}

/*
** Push a new element onto the priority queue
*/
static RtreeSearchPoint *rtreeEnqueue(
  RtreeCursor *pCur,    /* The cursor */
  RtreeDValue rScore,   /* Score for the new search point */
  u8 iLevel             /* Level for the new search point */
){
  int i, j;
  RtreeSearchPoint *pNew;
  if( pCur->nPoint>=pCur->nPointAlloc ){
    int nNew = pCur->nPointAlloc*2 + 8;
    pNew = sqlite3_realloc(pCur->aPoint, nNew*sizeof(pCur->aPoint[0]));
    if( pNew==0 ) return 0;
    pCur->aPoint = pNew;
    pCur->nPointAlloc = nNew;
  }
  i = pCur->nPoint++;
  pNew = pCur->aPoint + i;
  pNew->rScore = rScore;
  pNew->iLevel = iLevel;
  assert( iLevel>=0 && iLevel<=RTREE_MAX_DEPTH );
  while( i>0 ){
    RtreeSearchPoint *pParent;
    j = (i-1)/2;
    pParent = pCur->aPoint + j;
    if( rtreeSearchPointCompare(pNew, pParent)>=0 ) break;
    rtreeSearchPointSwap(pCur, j, i);
    i = j;
    pNew = pParent;
  }
  return pNew;
}

/*
** Allocate a new RtreeSearchPoint and return a pointer to it.  Return
** NULL if malloc fails.
*/
static RtreeSearchPoint *rtreeSearchPointNew(
  RtreeCursor *pCur,    /* The cursor */
  RtreeDValue rScore,   /* Score for the new search point */
  u8 iLevel             /* Level for the new search point */
){
  RtreeSearchPoint *pNew, *pFirst;
  pFirst = rtreeSearchPointFirst(pCur);
  pCur->anQueue[iLevel]++;
  if( pFirst==0
   || pFirst->rScore>rScore 
   || (pFirst->rScore==rScore && pFirst->iLevel>iLevel)
  ){
    if( pCur->bPoint ){
      int ii;
      pNew = rtreeEnqueue(pCur, rScore, iLevel);
      if( pNew==0 ) return 0;
      ii = (int)(pNew - pCur->aPoint) + 1;
      if( ii<RTREE_CACHE_SZ ){
        assert( pCur->aNode[ii]==0 );
        pCur->aNode[ii] = pCur->aNode[0];
       }else{
        nodeRelease(RTREE_OF_CURSOR(pCur), pCur->aNode[0]);
      }
      pCur->aNode[0] = 0;
      *pNew = pCur->sPoint;
    }
    pCur->sPoint.rScore = rScore;
    pCur->sPoint.iLevel = iLevel;
    pCur->bPoint = 1;
    return &pCur->sPoint;
  }else{
    return rtreeEnqueue(pCur, rScore, iLevel);
  }
}

#if 0
/* Tracing routines for the RtreeSearchPoint queue */
static void tracePoint(RtreeSearchPoint *p, int idx, RtreeCursor *pCur){
  if( idx<0 ){ printf(" s"); }else{ printf("%2d", idx); }
  printf(" %d.%05lld.%02d %g %d",
    p->iLevel, p->id, p->iCell, p->rScore, p->eWithin
  );
  idx++;
  if( idx<RTREE_CACHE_SZ ){
    printf(" %p\n", pCur->aNode[idx]);
  }else{
    printf("\n");
  }
}
static void traceQueue(RtreeCursor *pCur, const char *zPrefix){
  int ii;
  printf("=== %9s ", zPrefix);
  if( pCur->bPoint ){
    tracePoint(&pCur->sPoint, -1, pCur);
  }
  for(ii=0; ii<pCur->nPoint; ii++){
    if( ii>0 || pCur->bPoint ) printf("              ");
    tracePoint(&pCur->aPoint[ii], ii, pCur);
  }
}
# define RTREE_QUEUE_TRACE(A,B) traceQueue(A,B)
#else
# define RTREE_QUEUE_TRACE(A,B)   /* no-op */
#endif

/* Remove the search point with the lowest current score.
*/
static void rtreeSearchPointPop(RtreeCursor *p){
  int i, j, k, n;
  i = 1 - p->bPoint;
  assert( i==0 || i==1 );
  if( p->aNode[i] ){
    nodeRelease(RTREE_OF_CURSOR(p), p->aNode[i]);
    p->aNode[i] = 0;
  }
  if( p->bPoint ){
    p->anQueue[p->sPoint.iLevel]--;
    p->bPoint = 0;
  }else if( p->nPoint ){
    p->anQueue[p->aPoint[0].iLevel]--;
    n = --p->nPoint;
    p->aPoint[0] = p->aPoint[n];
    if( n<RTREE_CACHE_SZ-1 ){
      p->aNode[1] = p->aNode[n+1];
      p->aNode[n+1] = 0;
    }
    i = 0;
    while( (j = i*2+1)<n ){
      k = j+1;
      if( k<n && rtreeSearchPointCompare(&p->aPoint[k], &p->aPoint[j])<0 ){
        if( rtreeSearchPointCompare(&p->aPoint[k], &p->aPoint[i])<0 ){
          rtreeSearchPointSwap(p, i, k);
          i = k;
        }else{
          break;
        }
      }else{
        if( rtreeSearchPointCompare(&p->aPoint[j], &p->aPoint[i])<0 ){
          rtreeSearchPointSwap(p, i, j);
          i = j;
        }else{
          break;
        }
      }
    }
  }
}


/*
** Continue the search on cursor pCur until the front of the queue
** contains an entry suitable for returning as a result-set row,
** or until the RtreeSearchPoint queue is empty, indicating that the
** query has completed.
*/
static int rtreeStepToLeaf(RtreeCursor *pCur){
  RtreeSearchPoint *p;
  Rtree *pRtree = RTREE_OF_CURSOR(pCur);
  RtreeNode *pNode;
  int eWithin;
  int rc = SQLITE_OK;
  int nCell;
  int nConstraint = pCur->nConstraint;
  int ii;
  int eInt;
  RtreeSearchPoint x;

  eInt = pRtree->eCoordType==RTREE_COORD_INT32;
  while( (p = rtreeSearchPointFirst(pCur))!=0 && p->iLevel>0 ){
    pNode = rtreeNodeOfFirstSearchPoint(pCur, &rc);
    if( rc ) return rc;
    nCell = NCELL(pNode);
    assert( nCell<200 );
    while( p->iCell<nCell ){
      sqlite3_rtree_dbl rScore = (sqlite3_rtree_dbl)-1;
      u8 *pCellData = pNode->zData + (4+pRtree->nBytesPerCell*p->iCell);
      eWithin = FULLY_WITHIN;
      for(ii=0; ii<nConstraint; ii++){
        RtreeConstraint *pConstraint = pCur->aConstraint + ii;
        if( pConstraint->op>=RTREE_MATCH ){
          rc = rtreeCallbackConstraint(pConstraint, eInt, pCellData, p,
                                       &rScore, &eWithin);
          if( rc ) return rc;
        }else if( p->iLevel==1 ){
          rtreeLeafConstraint(pConstraint, eInt, pCellData, &eWithin);
        }else{
          rtreeNonleafConstraint(pConstraint, eInt, pCellData, &eWithin);
        }
        if( eWithin==NOT_WITHIN ) break;
      }
      p->iCell++;
      if( eWithin==NOT_WITHIN ) continue;
      x.iLevel = p->iLevel - 1;
      if( x.iLevel ){
        x.id = readInt64(pCellData);
        x.iCell = 0;
      }else{
        x.id = p->id;
        x.iCell = p->iCell - 1;
      }
      if( p->iCell>=nCell ){
        RTREE_QUEUE_TRACE(pCur, "POP-S:");
        rtreeSearchPointPop(pCur);
      }
      if( rScore<RTREE_ZERO ) rScore = RTREE_ZERO;
      p = rtreeSearchPointNew(pCur, rScore, x.iLevel);
      if( p==0 ) return SQLITE_NOMEM;
      p->eWithin = eWithin;
      p->id = x.id;
      p->iCell = x.iCell;
      RTREE_QUEUE_TRACE(pCur, "PUSH-S:");
      break;
    }
    if( p->iCell>=nCell ){
      RTREE_QUEUE_TRACE(pCur, "POP-Se:");
      rtreeSearchPointPop(pCur);
    }
  }
  pCur->atEOF = p==0;
  return SQLITE_OK;
}

/* 
** Rtree virtual table module xNext method.
*/
static int rtreeNext(sqlite3_vtab_cursor *pVtabCursor){
  RtreeCursor *pCsr = (RtreeCursor *)pVtabCursor;
  int rc = SQLITE_OK;

  /* Move to the next entry that matches the configured constraints. */
  RTREE_QUEUE_TRACE(pCsr, "POP-Nx:");
  rtreeSearchPointPop(pCsr);
  rc = rtreeStepToLeaf(pCsr);
  return rc;
}

/* 
** Rtree virtual table module xRowid method.
*/
static int rtreeRowid(sqlite3_vtab_cursor *pVtabCursor, sqlite_int64 *pRowid){
  RtreeCursor *pCsr = (RtreeCursor *)pVtabCursor;
  RtreeSearchPoint *p = rtreeSearchPointFirst(pCsr);
  int rc = SQLITE_OK;
  RtreeNode *pNode = rtreeNodeOfFirstSearchPoint(pCsr, &rc);
  if( rc==SQLITE_OK && p ){
    *pRowid = nodeGetRowid(RTREE_OF_CURSOR(pCsr), pNode, p->iCell);
  }
  return rc;
}

/* 
** Rtree virtual table module xColumn method.
*/
static int rtreeColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, int i){
  Rtree *pRtree = (Rtree *)cur->pVtab;
  RtreeCursor *pCsr = (RtreeCursor *)cur;
  RtreeSearchPoint *p = rtreeSearchPointFirst(pCsr);
  RtreeCoord c;
  int rc = SQLITE_OK;
  RtreeNode *pNode = rtreeNodeOfFirstSearchPoint(pCsr, &rc);

  if( rc ) return rc;
  if( p==0 ) return SQLITE_OK;
  if( i==0 ){
    sqlite3_result_int64(ctx, nodeGetRowid(pRtree, pNode, p->iCell));
  }else{
    if( rc ) return rc;
    nodeGetCoord(pRtree, pNode, p->iCell, i-1, &c);
#ifndef SQLITE_RTREE_INT_ONLY
    if( pRtree->eCoordType==RTREE_COORD_REAL32 ){
      sqlite3_result_double(ctx, c.f);
    }else
#endif
    {
      assert( pRtree->eCoordType==RTREE_COORD_INT32 );
      sqlite3_result_int(ctx, c.i);
    }
  }
  return SQLITE_OK;
}

/* 
** Use nodeAcquire() to obtain the leaf node containing the record with 
** rowid iRowid. If successful, set *ppLeaf to point to the node and
** return SQLITE_OK. If there is no such record in the table, set
** *ppLeaf to 0 and return SQLITE_OK. If an error occurs, set *ppLeaf
** to zero and return an SQLite error code.
*/
static int findLeafNode(
  Rtree *pRtree,              /* RTree to search */
  i64 iRowid,                 /* The rowid searching for */
  RtreeNode **ppLeaf,         /* Write the node here */
  sqlite3_int64 *piNode       /* Write the node-id here */
){
  int rc;
  *ppLeaf = 0;
  sqlite3_bind_int64(pRtree->pReadRowid, 1, iRowid);
  if( sqlite3_step(pRtree->pReadRowid)==SQLITE_ROW ){
    i64 iNode = sqlite3_column_int64(pRtree->pReadRowid, 0);
    if( piNode ) *piNode = iNode;
    rc = nodeAcquire(pRtree, iNode, 0, ppLeaf);
    sqlite3_reset(pRtree->pReadRowid);
  }else{
    rc = sqlite3_reset(pRtree->pReadRowid);
  }
  return rc;
}

/*
** This function is called to configure the RtreeConstraint object passed
** as the second argument for a MATCH constraint. The value passed as the
** first argument to this function is the right-hand operand to the MATCH
** operator.
*/
static int deserializeGeometry(sqlite3_value *pValue, RtreeConstraint *pCons){
  RtreeMatchArg *pBlob;              /* BLOB returned by geometry function */
  sqlite3_rtree_query_info *pInfo;   /* Callback information */
  int nBlob;                         /* Size of the geometry function blob */
  int nExpected;                     /* Expected size of the BLOB */

  /* Check that value is actually a blob. */
  if( sqlite3_value_type(pValue)!=SQLITE_BLOB ) return SQLITE_ERROR;

  /* Check that the blob is roughly the right size. */
  nBlob = sqlite3_value_bytes(pValue);
  if( nBlob<(int)sizeof(RtreeMatchArg) 
   || ((nBlob-sizeof(RtreeMatchArg))%sizeof(RtreeDValue))!=0
  ){
    return SQLITE_ERROR;
  }

  pInfo = (sqlite3_rtree_query_info*)sqlite3_malloc( sizeof(*pInfo)+nBlob );
  if( !pInfo ) return SQLITE_NOMEM;
  memset(pInfo, 0, sizeof(*pInfo));
  pBlob = (RtreeMatchArg*)&pInfo[1];

  memcpy(pBlob, sqlite3_value_blob(pValue), nBlob);
  nExpected = (int)(sizeof(RtreeMatchArg) +
                    (pBlob->nParam-1)*sizeof(RtreeDValue));
  if( pBlob->magic!=RTREE_GEOMETRY_MAGIC || nBlob!=nExpected ){
    sqlite3_free(pInfo);
    return SQLITE_ERROR;
  }
  pInfo->pContext = pBlob->cb.pContext;
  pInfo->nParam = pBlob->nParam;
  pInfo->aParam = pBlob->aParam;

  if( pBlob->cb.xGeom ){
    pCons->u.xGeom = pBlob->cb.xGeom;
  }else{
    pCons->op = RTREE_QUERY;
    pCons->u.xQueryFunc = pBlob->cb.xQueryFunc;
  }
  pCons->pInfo = pInfo;
  return SQLITE_OK;
}

/* 
** Rtree virtual table module xFilter method.
*/
static int rtreeFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  int idxNum, const char *idxStr,
  int argc, sqlite3_value **argv
){
  Rtree *pRtree = (Rtree *)pVtabCursor->pVtab;
  RtreeCursor *pCsr = (RtreeCursor *)pVtabCursor;
  RtreeNode *pRoot = 0;
  int ii;
  int rc = SQLITE_OK;
  int iCell = 0;

  rtreeReference(pRtree);

  /* Reset the cursor to the same state as rtreeOpen() leaves it in. */
  freeCursorConstraints(pCsr);
  sqlite3_free(pCsr->aPoint);
  memset(pCsr, 0, sizeof(RtreeCursor));
  pCsr->base.pVtab = (sqlite3_vtab*)pRtree;

  pCsr->iStrategy = idxNum;
  if( idxNum==1 ){
    /* Special case - lookup by rowid. */
    RtreeNode *pLeaf;        /* Leaf on which the required cell resides */
    RtreeSearchPoint *p;     /* Search point for the the leaf */
    i64 iRowid = sqlite3_value_int64(argv[0]);
    i64 iNode = 0;
    rc = findLeafNode(pRtree, iRowid, &pLeaf, &iNode);
    if( rc==SQLITE_OK && pLeaf!=0 ){
      p = rtreeSearchPointNew(pCsr, RTREE_ZERO, 0);
      assert( p!=0 );  /* Always returns pCsr->sPoint */
      pCsr->aNode[0] = pLeaf;
      p->id = iNode;
      p->eWithin = PARTLY_WITHIN;
      rc = nodeRowidIndex(pRtree, pLeaf, iRowid, &iCell);
      p->iCell = iCell;
      RTREE_QUEUE_TRACE(pCsr, "PUSH-F1:");
    }else{
      pCsr->atEOF = 1;
    }
  }else{
    /* Normal case - r-tree scan. Set up the RtreeCursor.aConstraint array 
    ** with the configured constraints. 
    */
    rc = nodeAcquire(pRtree, 1, 0, &pRoot);
    if( rc==SQLITE_OK && argc>0 ){
      pCsr->aConstraint = sqlite3_malloc(sizeof(RtreeConstraint)*argc);
      pCsr->nConstraint = argc;
      if( !pCsr->aConstraint ){
        rc = SQLITE_NOMEM;
      }else{
        memset(pCsr->aConstraint, 0, sizeof(RtreeConstraint)*argc);
        memset(pCsr->anQueue, 0, sizeof(u32)*(pRtree->iDepth + 1));
        assert( (idxStr==0 && argc==0)
                || (idxStr && (int)strlen(idxStr)==argc*2) );
        for(ii=0; ii<argc; ii++){
          RtreeConstraint *p = &pCsr->aConstraint[ii];
          p->op = idxStr[ii*2];
          p->iCoord = idxStr[ii*2+1]-'0';
          if( p->op>=RTREE_MATCH ){
            /* A MATCH operator. The right-hand-side must be a blob that
            ** can be cast into an RtreeMatchArg object. One created using
            ** an sqlite3_rtree_geometry_callback() SQL user function.
            */
            rc = deserializeGeometry(argv[ii], p);
            if( rc!=SQLITE_OK ){
              break;
            }
            p->pInfo->nCoord = pRtree->nDim*2;
            p->pInfo->anQueue = pCsr->anQueue;
            p->pInfo->mxLevel = pRtree->iDepth + 1;
          }else{
#ifdef SQLITE_RTREE_INT_ONLY
            p->u.rValue = sqlite3_value_int64(argv[ii]);
#else
            p->u.rValue = sqlite3_value_double(argv[ii]);
#endif
          }
        }
      }
    }
    if( rc==SQLITE_OK ){
      RtreeSearchPoint *pNew;
      pNew = rtreeSearchPointNew(pCsr, RTREE_ZERO, pRtree->iDepth+1);
      if( pNew==0 ) return SQLITE_NOMEM;
      pNew->id = 1;
      pNew->iCell = 0;
      pNew->eWithin = PARTLY_WITHIN;
      assert( pCsr->bPoint==1 );
      pCsr->aNode[0] = pRoot;
      pRoot = 0;
      RTREE_QUEUE_TRACE(pCsr, "PUSH-Fm:");
      rc = rtreeStepToLeaf(pCsr);
    }
  }

  nodeRelease(pRtree, pRoot);
  rtreeRelease(pRtree);
  return rc;
}

/*
** Set the pIdxInfo->estimatedRows variable to nRow. Unless this
** extension is currently being used by a version of SQLite too old to
** support estimatedRows. In that case this function is a no-op.
*/
static void setEstimatedRows(sqlite3_index_info *pIdxInfo, i64 nRow){
#if SQLITE_VERSION_NUMBER>=3008002
  if( sqlite3_libversion_number()>=3008002 ){
    pIdxInfo->estimatedRows = nRow;
  }
#endif
}

/*
** Rtree virtual table module xBestIndex method. There are three
** table scan strategies to choose from (in order from most to 
** least desirable):
**
**   idxNum     idxStr        Strategy
**   ------------------------------------------------
**     1        Unused        Direct lookup by rowid.
**     2        See below     R-tree query or full-table scan.
**   ------------------------------------------------
**
** If strategy 1 is used, then idxStr is not meaningful. If strategy
** 2 is used, idxStr is formatted to contain 2 bytes for each 
** constraint used. The first two bytes of idxStr correspond to 
** the constraint in sqlite3_index_info.aConstraintUsage[] with
** (argvIndex==1) etc.
**
** The first of each pair of bytes in idxStr identifies the constraint
** operator as follows:
**
**   Operator    Byte Value
**   ----------------------
**      =        0x41 ('A')
**     <=        0x42 ('B')
**      <        0x43 ('C')
**     >=        0x44 ('D')
**      >        0x45 ('E')
**   MATCH       0x46 ('F')
**   ----------------------
**
** The second of each pair of bytes identifies the coordinate column
** to which the constraint applies. The leftmost coordinate column
** is 'a', the second from the left 'b' etc.
*/
static int rtreeBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  Rtree *pRtree = (Rtree*)tab;
  int rc = SQLITE_OK;
  int ii;
  i64 nRow;                       /* Estimated rows returned by this scan */

  int iIdx = 0;
  char zIdxStr[RTREE_MAX_DIMENSIONS*8+1];
  memset(zIdxStr, 0, sizeof(zIdxStr));

  assert( pIdxInfo->idxStr==0 );
  for(ii=0; ii<pIdxInfo->nConstraint && iIdx<(int)(sizeof(zIdxStr)-1); ii++){
    struct sqlite3_index_constraint *p = &pIdxInfo->aConstraint[ii];

    if( p->usable && p->iColumn==0 && p->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      /* We have an equality constraint on the rowid. Use strategy 1. */
      int jj;
      for(jj=0; jj<ii; jj++){
        pIdxInfo->aConstraintUsage[jj].argvIndex = 0;
        pIdxInfo->aConstraintUsage[jj].omit = 0;
      }
      pIdxInfo->idxNum = 1;
      pIdxInfo->aConstraintUsage[ii].argvIndex = 1;
      pIdxInfo->aConstraintUsage[jj].omit = 1;

      /* This strategy involves a two rowid lookups on an B-Tree structures
      ** and then a linear search of an R-Tree node. This should be 
      ** considered almost as quick as a direct rowid lookup (for which 
      ** sqlite uses an internal cost of 0.0). It is expected to return
      ** a single row.
      */ 
      pIdxInfo->estimatedCost = 30.0;
      setEstimatedRows(pIdxInfo, 1);
      return SQLITE_OK;
    }

    if( p->usable && (p->iColumn>0 || p->op==SQLITE_INDEX_CONSTRAINT_MATCH) ){
      u8 op;
      switch( p->op ){
        case SQLITE_INDEX_CONSTRAINT_EQ: op = RTREE_EQ; break;
        case SQLITE_INDEX_CONSTRAINT_GT: op = RTREE_GT; break;
        case SQLITE_INDEX_CONSTRAINT_LE: op = RTREE_LE; break;
        case SQLITE_INDEX_CONSTRAINT_LT: op = RTREE_LT; break;
        case SQLITE_INDEX_CONSTRAINT_GE: op = RTREE_GE; break;
        default:
          assert( p->op==SQLITE_INDEX_CONSTRAINT_MATCH );
          op = RTREE_MATCH; 
          break;
      }
      zIdxStr[iIdx++] = op;
      zIdxStr[iIdx++] = p->iColumn - 1 + '0';
      pIdxInfo->aConstraintUsage[ii].argvIndex = (iIdx/2);
      pIdxInfo->aConstraintUsage[ii].omit = 1;
    }
  }

  pIdxInfo->idxNum = 2;
  pIdxInfo->needToFreeIdxStr = 1;
  if( iIdx>0 && 0==(pIdxInfo->idxStr = sqlite3_mprintf("%s", zIdxStr)) ){
    return SQLITE_NOMEM;
  }

  nRow = pRtree->nRowEst / (iIdx + 1);
  pIdxInfo->estimatedCost = (double)6.0 * (double)nRow;
  setEstimatedRows(pIdxInfo, nRow);

  return rc;
}

/*
** Return the N-dimensional volumn of the cell stored in *p.
*/
static RtreeDValue cellArea(Rtree *pRtree, RtreeCell *p){
  RtreeDValue area = (RtreeDValue)1;
  int ii;
  for(ii=0; ii<(pRtree->nDim*2); ii+=2){
    area = (area * (DCOORD(p->aCoord[ii+1]) - DCOORD(p->aCoord[ii])));
  }
  return area;
}

/*
** Return the margin length of cell p. The margin length is the sum
** of the objects size in each dimension.
*/
static RtreeDValue cellMargin(Rtree *pRtree, RtreeCell *p){
  RtreeDValue margin = (RtreeDValue)0;
  int ii;
  for(ii=0; ii<(pRtree->nDim*2); ii+=2){
    margin += (DCOORD(p->aCoord[ii+1]) - DCOORD(p->aCoord[ii]));
  }
  return margin;
}

/*
** Store the union of cells p1 and p2 in p1.
*/
static void cellUnion(Rtree *pRtree, RtreeCell *p1, RtreeCell *p2){
  int ii;
  if( pRtree->eCoordType==RTREE_COORD_REAL32 ){
    for(ii=0; ii<(pRtree->nDim*2); ii+=2){
      p1->aCoord[ii].f = MIN(p1->aCoord[ii].f, p2->aCoord[ii].f);
      p1->aCoord[ii+1].f = MAX(p1->aCoord[ii+1].f, p2->aCoord[ii+1].f);
    }
  }else{
    for(ii=0; ii<(pRtree->nDim*2); ii+=2){
      p1->aCoord[ii].i = MIN(p1->aCoord[ii].i, p2->aCoord[ii].i);
      p1->aCoord[ii+1].i = MAX(p1->aCoord[ii+1].i, p2->aCoord[ii+1].i);
    }
  }
}

/*
** Return true if the area covered by p2 is a subset of the area covered
** by p1. False otherwise.
*/
static int cellContains(Rtree *pRtree, RtreeCell *p1, RtreeCell *p2){
  int ii;
  int isInt = (pRtree->eCoordType==RTREE_COORD_INT32);
  for(ii=0; ii<(pRtree->nDim*2); ii+=2){
    RtreeCoord *a1 = &p1->aCoord[ii];
    RtreeCoord *a2 = &p2->aCoord[ii];
    if( (!isInt && (a2[0].f<a1[0].f || a2[1].f>a1[1].f)) 
     || ( isInt && (a2[0].i<a1[0].i || a2[1].i>a1[1].i)) 
    ){
      return 0;
    }
  }
  return 1;
}

/*
** Return the amount cell p would grow by if it were unioned with pCell.
*/
static RtreeDValue cellGrowth(Rtree *pRtree, RtreeCell *p, RtreeCell *pCell){
  RtreeDValue area;
  RtreeCell cell;
  memcpy(&cell, p, sizeof(RtreeCell));
  area = cellArea(pRtree, &cell);
  cellUnion(pRtree, &cell, pCell);
  return (cellArea(pRtree, &cell)-area);
}

static RtreeDValue cellOverlap(
  Rtree *pRtree, 
  RtreeCell *p, 
  RtreeCell *aCell, 
  int nCell
){
  int ii;
  RtreeDValue overlap = RTREE_ZERO;
  for(ii=0; ii<nCell; ii++){
    int jj;
    RtreeDValue o = (RtreeDValue)1;
    for(jj=0; jj<(pRtree->nDim*2); jj+=2){
      RtreeDValue x1, x2;
      x1 = MAX(DCOORD(p->aCoord[jj]), DCOORD(aCell[ii].aCoord[jj]));
      x2 = MIN(DCOORD(p->aCoord[jj+1]), DCOORD(aCell[ii].aCoord[jj+1]));
      if( x2<x1 ){
        o = (RtreeDValue)0;
        break;
      }else{
        o = o * (x2-x1);
      }
    }
    overlap += o;
  }
  return overlap;
}


/*
** This function implements the ChooseLeaf algorithm from Gutman[84].
** ChooseSubTree in r*tree terminology.
*/
static int ChooseLeaf(
  Rtree *pRtree,               /* Rtree table */
  RtreeCell *pCell,            /* Cell to insert into rtree */
  int iHeight,                 /* Height of sub-tree rooted at pCell */
  RtreeNode **ppLeaf           /* OUT: Selected leaf page */
){
  int rc;
  int ii;
  RtreeNode *pNode;
  rc = nodeAcquire(pRtree, 1, 0, &pNode);

  for(ii=0; rc==SQLITE_OK && ii<(pRtree->iDepth-iHeight); ii++){
    int iCell;
    sqlite3_int64 iBest = 0;

    RtreeDValue fMinGrowth = RTREE_ZERO;
    RtreeDValue fMinArea = RTREE_ZERO;

    int nCell = NCELL(pNode);
    RtreeCell cell;
    RtreeNode *pChild;

    RtreeCell *aCell = 0;

    /* Select the child node which will be enlarged the least if pCell
    ** is inserted into it. Resolve ties by choosing the entry with
    ** the smallest area.
    */
    for(iCell=0; iCell<nCell; iCell++){
      int bBest = 0;
      RtreeDValue growth;
      RtreeDValue area;
      nodeGetCell(pRtree, pNode, iCell, &cell);
      growth = cellGrowth(pRtree, &cell, pCell);
      area = cellArea(pRtree, &cell);
      if( iCell==0||growth<fMinGrowth||(growth==fMinGrowth && area<fMinArea) ){
        bBest = 1;
      }
      if( bBest ){
        fMinGrowth = growth;
        fMinArea = area;
        iBest = cell.iRowid;
      }
    }

    sqlite3_free(aCell);
    rc = nodeAcquire(pRtree, iBest, pNode, &pChild);
    nodeRelease(pRtree, pNode);
    pNode = pChild;
  }

  *ppLeaf = pNode;
  return rc;
}

/*
** A cell with the same content as pCell has just been inserted into
** the node pNode. This function updates the bounding box cells in
** all ancestor elements.
*/
static int AdjustTree(
  Rtree *pRtree,                    /* Rtree table */
  RtreeNode *pNode,                 /* Adjust ancestry of this node. */
  RtreeCell *pCell                  /* This cell was just inserted */
){
  RtreeNode *p = pNode;
  while( p->pParent ){
    RtreeNode *pParent = p->pParent;
    RtreeCell cell;
    int iCell;

    if( nodeParentIndex(pRtree, p, &iCell) ){
      return SQLITE_CORRUPT_VTAB;
    }

    nodeGetCell(pRtree, pParent, iCell, &cell);
    if( !cellContains(pRtree, &cell, pCell) ){
      cellUnion(pRtree, &cell, pCell);
      nodeOverwriteCell(pRtree, pParent, &cell, iCell);
    }
 
    p = pParent;
  }
  return SQLITE_OK;
}

/*
** Write mapping (iRowid->iNode) to the <rtree>_rowid table.
*/
static int rowidWrite(Rtree *pRtree, sqlite3_int64 iRowid, sqlite3_int64 iNode){
  sqlite3_bind_int64(pRtree->pWriteRowid, 1, iRowid);
  sqlite3_bind_int64(pRtree->pWriteRowid, 2, iNode);
  sqlite3_step(pRtree->pWriteRowid);
  return sqlite3_reset(pRtree->pWriteRowid);
}

/*
** Write mapping (iNode->iPar) to the <rtree>_parent table.
*/
static int parentWrite(Rtree *pRtree, sqlite3_int64 iNode, sqlite3_int64 iPar){
  sqlite3_bind_int64(pRtree->pWriteParent, 1, iNode);
  sqlite3_bind_int64(pRtree->pWriteParent, 2, iPar);
  sqlite3_step(pRtree->pWriteParent);
  return sqlite3_reset(pRtree->pWriteParent);
}

static int rtreeInsertCell(Rtree *, RtreeNode *, RtreeCell *, int);


/*
** Arguments aIdx, aDistance and aSpare all point to arrays of size
** nIdx. The aIdx array contains the set of integers from 0 to 
** (nIdx-1) in no particular order. This function sorts the values
** in aIdx according to the indexed values in aDistance. For
** example, assuming the inputs:
**
**   aIdx      = { 0,   1,   2,   3 }
**   aDistance = { 5.0, 2.0, 7.0, 6.0 }
**
** this function sets the aIdx array to contain:
**
**   aIdx      = { 0,   1,   2,   3 }
**
** The aSpare array is used as temporary working space by the
** sorting algorithm.
*/
static void SortByDistance(
  int *aIdx, 
  int nIdx, 
  RtreeDValue *aDistance, 
  int *aSpare
){
  if( nIdx>1 ){
    int iLeft = 0;
    int iRight = 0;

    int nLeft = nIdx/2;
    int nRight = nIdx-nLeft;
    int *aLeft = aIdx;
    int *aRight = &aIdx[nLeft];

    SortByDistance(aLeft, nLeft, aDistance, aSpare);
    SortByDistance(aRight, nRight, aDistance, aSpare);

    memcpy(aSpare, aLeft, sizeof(int)*nLeft);
    aLeft = aSpare;

    while( iLeft<nLeft || iRight<nRight ){
      if( iLeft==nLeft ){
        aIdx[iLeft+iRight] = aRight[iRight];
        iRight++;
      }else if( iRight==nRight ){
        aIdx[iLeft+iRight] = aLeft[iLeft];
        iLeft++;
      }else{
        RtreeDValue fLeft = aDistance[aLeft[iLeft]];
        RtreeDValue fRight = aDistance[aRight[iRight]];
        if( fLeft<fRight ){
          aIdx[iLeft+iRight] = aLeft[iLeft];
          iLeft++;
        }else{
          aIdx[iLeft+iRight] = aRight[iRight];
          iRight++;
        }
      }
    }

#if 0
    /* Check that the sort worked */
    {
      int jj;
      for(jj=1; jj<nIdx; jj++){
        RtreeDValue left = aDistance[aIdx[jj-1]];
        RtreeDValue right = aDistance[aIdx[jj]];
        assert( left<=right );
      }
    }
#endif
  }
}

/*
** Arguments aIdx, aCell and aSpare all point to arrays of size
** nIdx. The aIdx array contains the set of integers from 0 to 
** (nIdx-1) in no particular order. This function sorts the values
** in aIdx according to dimension iDim of the cells in aCell. The
** minimum value of dimension iDim is considered first, the
** maximum used to break ties.
**
** The aSpare array is used as temporary working space by the
** sorting algorithm.
*/
static void SortByDimension(
  Rtree *pRtree,
  int *aIdx, 
  int nIdx, 
  int iDim, 
  RtreeCell *aCell, 
  int *aSpare
){
  if( nIdx>1 ){

    int iLeft = 0;
    int iRight = 0;

    int nLeft = nIdx/2;
    int nRight = nIdx-nLeft;
    int *aLeft = aIdx;
    int *aRight = &aIdx[nLeft];

    SortByDimension(pRtree, aLeft, nLeft, iDim, aCell, aSpare);
    SortByDimension(pRtree, aRight, nRight, iDim, aCell, aSpare);

    memcpy(aSpare, aLeft, sizeof(int)*nLeft);
    aLeft = aSpare;
    while( iLeft<nLeft || iRight<nRight ){
      RtreeDValue xleft1 = DCOORD(aCell[aLeft[iLeft]].aCoord[iDim*2]);
      RtreeDValue xleft2 = DCOORD(aCell[aLeft[iLeft]].aCoord[iDim*2+1]);
      RtreeDValue xright1 = DCOORD(aCell[aRight[iRight]].aCoord[iDim*2]);
      RtreeDValue xright2 = DCOORD(aCell[aRight[iRight]].aCoord[iDim*2+1]);
      if( (iLeft!=nLeft) && ((iRight==nRight)
       || (xleft1<xright1)
       || (xleft1==xright1 && xleft2<xright2)
      )){
        aIdx[iLeft+iRight] = aLeft[iLeft];
        iLeft++;
      }else{
        aIdx[iLeft+iRight] = aRight[iRight];
        iRight++;
      }
    }

#if 0
    /* Check that the sort worked */
    {
      int jj;
      for(jj=1; jj<nIdx; jj++){
        RtreeDValue xleft1 = aCell[aIdx[jj-1]].aCoord[iDim*2];
        RtreeDValue xleft2 = aCell[aIdx[jj-1]].aCoord[iDim*2+1];
        RtreeDValue xright1 = aCell[aIdx[jj]].aCoord[iDim*2];
        RtreeDValue xright2 = aCell[aIdx[jj]].aCoord[iDim*2+1];
        assert( xleft1<=xright1 && (xleft1<xright1 || xleft2<=xright2) );
      }
    }
#endif
  }
}

/*
** Implementation of the R*-tree variant of SplitNode from Beckman[1990].
*/
static int splitNodeStartree(
  Rtree *pRtree,
  RtreeCell *aCell,
  int nCell,
  RtreeNode *pLeft,
  RtreeNode *pRight,
  RtreeCell *pBboxLeft,
  RtreeCell *pBboxRight
){
  int **aaSorted;
  int *aSpare;
  int ii;

  int iBestDim = 0;
  int iBestSplit = 0;
  RtreeDValue fBestMargin = RTREE_ZERO;

  int nByte = (pRtree->nDim+1)*(sizeof(int*)+nCell*sizeof(int));

  aaSorted = (int **)sqlite3_malloc(nByte);
  if( !aaSorted ){
    return SQLITE_NOMEM;
  }

  aSpare = &((int *)&aaSorted[pRtree->nDim])[pRtree->nDim*nCell];
  memset(aaSorted, 0, nByte);
  for(ii=0; ii<pRtree->nDim; ii++){
    int jj;
    aaSorted[ii] = &((int *)&aaSorted[pRtree->nDim])[ii*nCell];
    for(jj=0; jj<nCell; jj++){
      aaSorted[ii][jj] = jj;
    }
    SortByDimension(pRtree, aaSorted[ii], nCell, ii, aCell, aSpare);
  }

  for(ii=0; ii<pRtree->nDim; ii++){
    RtreeDValue margin = RTREE_ZERO;
    RtreeDValue fBestOverlap = RTREE_ZERO;
    RtreeDValue fBestArea = RTREE_ZERO;
    int iBestLeft = 0;
    int nLeft;

    for(
      nLeft=RTREE_MINCELLS(pRtree); 
      nLeft<=(nCell-RTREE_MINCELLS(pRtree)); 
      nLeft++
    ){
      RtreeCell left;
      RtreeCell right;
      int kk;
      RtreeDValue overlap;
      RtreeDValue area;

      memcpy(&left, &aCell[aaSorted[ii][0]], sizeof(RtreeCell));
      memcpy(&right, &aCell[aaSorted[ii][nCell-1]], sizeof(RtreeCell));
      for(kk=1; kk<(nCell-1); kk++){
        if( kk<nLeft ){
          cellUnion(pRtree, &left, &aCell[aaSorted[ii][kk]]);
        }else{
          cellUnion(pRtree, &right, &aCell[aaSorted[ii][kk]]);
        }
      }
      margin += cellMargin(pRtree, &left);
      margin += cellMargin(pRtree, &right);
      overlap = cellOverlap(pRtree, &left, &right, 1);
      area = cellArea(pRtree, &left) + cellArea(pRtree, &right);
      if( (nLeft==RTREE_MINCELLS(pRtree))
       || (overlap<fBestOverlap)
       || (overlap==fBestOverlap && area<fBestArea)
      ){
        iBestLeft = nLeft;
        fBestOverlap = overlap;
        fBestArea = area;
      }
    }

    if( ii==0 || margin<fBestMargin ){
      iBestDim = ii;
      fBestMargin = margin;
      iBestSplit = iBestLeft;
    }
  }

  memcpy(pBboxLeft, &aCell[aaSorted[iBestDim][0]], sizeof(RtreeCell));
  memcpy(pBboxRight, &aCell[aaSorted[iBestDim][iBestSplit]], sizeof(RtreeCell));
  for(ii=0; ii<nCell; ii++){
    RtreeNode *pTarget = (ii<iBestSplit)?pLeft:pRight;
    RtreeCell *pBbox = (ii<iBestSplit)?pBboxLeft:pBboxRight;
    RtreeCell *pCell = &aCell[aaSorted[iBestDim][ii]];
    nodeInsertCell(pRtree, pTarget, pCell);
    cellUnion(pRtree, pBbox, pCell);
  }

  sqlite3_free(aaSorted);
  return SQLITE_OK;
}


static int updateMapping(
  Rtree *pRtree, 
  i64 iRowid, 
  RtreeNode *pNode, 
  int iHeight
){
  int (*xSetMapping)(Rtree *, sqlite3_int64, sqlite3_int64);
  xSetMapping = ((iHeight==0)?rowidWrite:parentWrite);
  if( iHeight>0 ){
    RtreeNode *pChild = nodeHashLookup(pRtree, iRowid);
    if( pChild ){
      nodeRelease(pRtree, pChild->pParent);
      nodeReference(pNode);
      pChild->pParent = pNode;
    }
  }
  return xSetMapping(pRtree, iRowid, pNode->iNode);
}

static int SplitNode(
  Rtree *pRtree,
  RtreeNode *pNode,
  RtreeCell *pCell,
  int iHeight
){
  int i;
  int newCellIsRight = 0;

  int rc = SQLITE_OK;
  int nCell = NCELL(pNode);
  RtreeCell *aCell;
  int *aiUsed;

  RtreeNode *pLeft = 0;
  RtreeNode *pRight = 0;

  RtreeCell leftbbox;
  RtreeCell rightbbox;

  /* Allocate an array and populate it with a copy of pCell and 
  ** all cells from node pLeft. Then zero the original node.
  */
  aCell = sqlite3_malloc((sizeof(RtreeCell)+sizeof(int))*(nCell+1));
  if( !aCell ){
    rc = SQLITE_NOMEM;
    goto splitnode_out;
  }
  aiUsed = (int *)&aCell[nCell+1];
  memset(aiUsed, 0, sizeof(int)*(nCell+1));
  for(i=0; i<nCell; i++){
    nodeGetCell(pRtree, pNode, i, &aCell[i]);
  }
  nodeZero(pRtree, pNode);
  memcpy(&aCell[nCell], pCell, sizeof(RtreeCell));
  nCell++;

  if( pNode->iNode==1 ){
    pRight = nodeNew(pRtree, pNode);
    pLeft = nodeNew(pRtree, pNode);
    pRtree->iDepth++;
    pNode->isDirty = 1;
    writeInt16(pNode->zData, pRtree->iDepth);
  }else{
    pLeft = pNode;
    pRight = nodeNew(pRtree, pLeft->pParent);
    nodeReference(pLeft);
  }

  if( !pLeft || !pRight ){
    rc = SQLITE_NOMEM;
    goto splitnode_out;
  }

  memset(pLeft->zData, 0, pRtree->iNodeSize);
  memset(pRight->zData, 0, pRtree->iNodeSize);

  rc = splitNodeStartree(pRtree, aCell, nCell, pLeft, pRight,
                         &leftbbox, &rightbbox);
  if( rc!=SQLITE_OK ){
    goto splitnode_out;
  }

  /* Ensure both child nodes have node numbers assigned to them by calling
  ** nodeWrite(). Node pRight always needs a node number, as it was created
  ** by nodeNew() above. But node pLeft sometimes already has a node number.
  ** In this case avoid the all to nodeWrite().
  */
  if( SQLITE_OK!=(rc = nodeWrite(pRtree, pRight))
   || (0==pLeft->iNode && SQLITE_OK!=(rc = nodeWrite(pRtree, pLeft)))
  ){
    goto splitnode_out;
  }

  rightbbox.iRowid = pRight->iNode;
  leftbbox.iRowid = pLeft->iNode;

  if( pNode->iNode==1 ){
    rc = rtreeInsertCell(pRtree, pLeft->pParent, &leftbbox, iHeight+1);
    if( rc!=SQLITE_OK ){
      goto splitnode_out;
    }
  }else{
    RtreeNode *pParent = pLeft->pParent;
    int iCell;
    rc = nodeParentIndex(pRtree, pLeft, &iCell);
    if( rc==SQLITE_OK ){
      nodeOverwriteCell(pRtree, pParent, &leftbbox, iCell);
      rc = AdjustTree(pRtree, pParent, &leftbbox);
    }
    if( rc!=SQLITE_OK ){
      goto splitnode_out;
    }
  }
  if( (rc = rtreeInsertCell(pRtree, pRight->pParent, &rightbbox, iHeight+1)) ){
    goto splitnode_out;
  }

  for(i=0; i<NCELL(pRight); i++){
    i64 iRowid = nodeGetRowid(pRtree, pRight, i);
    rc = updateMapping(pRtree, iRowid, pRight, iHeight);
    if( iRowid==pCell->iRowid ){
      newCellIsRight = 1;
    }
    if( rc!=SQLITE_OK ){
      goto splitnode_out;
    }
  }
  if( pNode->iNode==1 ){
    for(i=0; i<NCELL(pLeft); i++){
      i64 iRowid = nodeGetRowid(pRtree, pLeft, i);
      rc = updateMapping(pRtree, iRowid, pLeft, iHeight);
      if( rc!=SQLITE_OK ){
        goto splitnode_out;
      }
    }
  }else if( newCellIsRight==0 ){
    rc = updateMapping(pRtree, pCell->iRowid, pLeft, iHeight);
  }

  if( rc==SQLITE_OK ){
    rc = nodeRelease(pRtree, pRight);
    pRight = 0;
  }
  if( rc==SQLITE_OK ){
    rc = nodeRelease(pRtree, pLeft);
    pLeft = 0;
  }

splitnode_out:
  nodeRelease(pRtree, pRight);
  nodeRelease(pRtree, pLeft);
  sqlite3_free(aCell);
  return rc;
}

/*
** If node pLeaf is not the root of the r-tree and its pParent pointer is 
** still NULL, load all ancestor nodes of pLeaf into memory and populate
** the pLeaf->pParent chain all the way up to the root node.
**
** This operation is required when a row is deleted (or updated - an update
** is implemented as a delete followed by an insert). SQLite provides the
** rowid of the row to delete, which can be used to find the leaf on which
** the entry resides (argument pLeaf). Once the leaf is located, this 
** function is called to determine its ancestry.
*/
static int fixLeafParent(Rtree *pRtree, RtreeNode *pLeaf){
  int rc = SQLITE_OK;
  RtreeNode *pChild = pLeaf;
  while( rc==SQLITE_OK && pChild->iNode!=1 && pChild->pParent==0 ){
    int rc2 = SQLITE_OK;          /* sqlite3_reset() return code */
    sqlite3_bind_int64(pRtree->pReadParent, 1, pChild->iNode);
    rc = sqlite3_step(pRtree->pReadParent);
    if( rc==SQLITE_ROW ){
      RtreeNode *pTest;           /* Used to test for reference loops */
      i64 iNode;                  /* Node number of parent node */

      /* Before setting pChild->pParent, test that we are not creating a
      ** loop of references (as we would if, say, pChild==pParent). We don't
      ** want to do this as it leads to a memory leak when trying to delete
      ** the referenced counted node structures.
      */
      iNode = sqlite3_column_int64(pRtree->pReadParent, 0);
      for(pTest=pLeaf; pTest && pTest->iNode!=iNode; pTest=pTest->pParent);
      if( !pTest ){
        rc2 = nodeAcquire(pRtree, iNode, 0, &pChild->pParent);
      }
    }
    rc = sqlite3_reset(pRtree->pReadParent);
    if( rc==SQLITE_OK ) rc = rc2;
    if( rc==SQLITE_OK && !pChild->pParent ) rc = SQLITE_CORRUPT_VTAB;
    pChild = pChild->pParent;
  }
  return rc;
}

static int deleteCell(Rtree *, RtreeNode *, int, int);

static int removeNode(Rtree *pRtree, RtreeNode *pNode, int iHeight){
  int rc;
  int rc2;
  RtreeNode *pParent = 0;
  int iCell;

  assert( pNode->nRef==1 );

  /* Remove the entry in the parent cell. */
  rc = nodeParentIndex(pRtree, pNode, &iCell);
  if( rc==SQLITE_OK ){
    pParent = pNode->pParent;
    pNode->pParent = 0;
    rc = deleteCell(pRtree, pParent, iCell, iHeight+1);
  }
  rc2 = nodeRelease(pRtree, pParent);
  if( rc==SQLITE_OK ){
    rc = rc2;
  }
  if( rc!=SQLITE_OK ){
    return rc;
  }

  /* Remove the xxx_node entry. */
  sqlite3_bind_int64(pRtree->pDeleteNode, 1, pNode->iNode);
  sqlite3_step(pRtree->pDeleteNode);
  if( SQLITE_OK!=(rc = sqlite3_reset(pRtree->pDeleteNode)) ){
    return rc;
  }

  /* Remove the xxx_parent entry. */
  sqlite3_bind_int64(pRtree->pDeleteParent, 1, pNode->iNode);
  sqlite3_step(pRtree->pDeleteParent);
  if( SQLITE_OK!=(rc = sqlite3_reset(pRtree->pDeleteParent)) ){
    return rc;
  }
  
  /* Remove the node from the in-memory hash table and link it into
  ** the Rtree.pDeleted list. Its contents will be re-inserted later on.
  */
  nodeHashDelete(pRtree, pNode);
  pNode->iNode = iHeight;
  pNode->pNext = pRtree->pDeleted;
  pNode->nRef++;
  pRtree->pDeleted = pNode;

  return SQLITE_OK;
}

static int fixBoundingBox(Rtree *pRtree, RtreeNode *pNode){
  RtreeNode *pParent = pNode->pParent;
  int rc = SQLITE_OK; 
  if( pParent ){
    int ii; 
    int nCell = NCELL(pNode);
    RtreeCell box;                            /* Bounding box for pNode */
    nodeGetCell(pRtree, pNode, 0, &box);
    for(ii=1; ii<nCell; ii++){
      RtreeCell cell;
      nodeGetCell(pRtree, pNode, ii, &cell);
      cellUnion(pRtree, &box, &cell);
    }
    box.iRowid = pNode->iNode;
    rc = nodeParentIndex(pRtree, pNode, &ii);
    if( rc==SQLITE_OK ){
      nodeOverwriteCell(pRtree, pParent, &box, ii);
      rc = fixBoundingBox(pRtree, pParent);
    }
  }
  return rc;
}

/*
** Delete the cell at index iCell of node pNode. After removing the
** cell, adjust the r-tree data structure if required.
*/
static int deleteCell(Rtree *pRtree, RtreeNode *pNode, int iCell, int iHeight){
  RtreeNode *pParent;
  int rc;

  if( SQLITE_OK!=(rc = fixLeafParent(pRtree, pNode)) ){
    return rc;
  }

  /* Remove the cell from the node. This call just moves bytes around
  ** the in-memory node image, so it cannot fail.
  */
  nodeDeleteCell(pRtree, pNode, iCell);

  /* If the node is not the tree root and now has less than the minimum
  ** number of cells, remove it from the tree. Otherwise, update the
  ** cell in the parent node so that it tightly contains the updated
  ** node.
  */
  pParent = pNode->pParent;
  assert( pParent || pNode->iNode==1 );
  if( pParent ){
    if( NCELL(pNode)<RTREE_MINCELLS(pRtree) ){
      rc = removeNode(pRtree, pNode, iHeight);
    }else{
      rc = fixBoundingBox(pRtree, pNode);
    }
  }

  return rc;
}

static int Reinsert(
  Rtree *pRtree, 
  RtreeNode *pNode, 
  RtreeCell *pCell, 
  int iHeight
){
  int *aOrder;
  int *aSpare;
  RtreeCell *aCell;
  RtreeDValue *aDistance;
  int nCell;
  RtreeDValue aCenterCoord[RTREE_MAX_DIMENSIONS];
  int iDim;
  int ii;
  int rc = SQLITE_OK;
  int n;

  memset(aCenterCoord, 0, sizeof(RtreeDValue)*RTREE_MAX_DIMENSIONS);

  nCell = NCELL(pNode)+1;
  n = (nCell+1)&(~1);

  /* Allocate the buffers used by this operation. The allocation is
  ** relinquished before this function returns.
  */
  aCell = (RtreeCell *)sqlite3_malloc(n * (
    sizeof(RtreeCell)     +         /* aCell array */
    sizeof(int)           +         /* aOrder array */
    sizeof(int)           +         /* aSpare array */
    sizeof(RtreeDValue)             /* aDistance array */
  ));
  if( !aCell ){
    return SQLITE_NOMEM;
  }
  aOrder    = (int *)&aCell[n];
  aSpare    = (int *)&aOrder[n];
  aDistance = (RtreeDValue *)&aSpare[n];

  for(ii=0; ii<nCell; ii++){
    if( ii==(nCell-1) ){
      memcpy(&aCell[ii], pCell, sizeof(RtreeCell));
    }else{
      nodeGetCell(pRtree, pNode, ii, &aCell[ii]);
    }
    aOrder[ii] = ii;
    for(iDim=0; iDim<pRtree->nDim; iDim++){
      aCenterCoord[iDim] += DCOORD(aCell[ii].aCoord[iDim*2]);
      aCenterCoord[iDim] += DCOORD(aCell[ii].aCoord[iDim*2+1]);
    }
  }
  for(iDim=0; iDim<pRtree->nDim; iDim++){
    aCenterCoord[iDim] = (aCenterCoord[iDim]/(nCell*(RtreeDValue)2));
  }

  for(ii=0; ii<nCell; ii++){
    aDistance[ii] = RTREE_ZERO;
    for(iDim=0; iDim<pRtree->nDim; iDim++){
      RtreeDValue coord = (DCOORD(aCell[ii].aCoord[iDim*2+1]) - 
                               DCOORD(aCell[ii].aCoord[iDim*2]));
      aDistance[ii] += (coord-aCenterCoord[iDim])*(coord-aCenterCoord[iDim]);
    }
  }

  SortByDistance(aOrder, nCell, aDistance, aSpare);
  nodeZero(pRtree, pNode);

  for(ii=0; rc==SQLITE_OK && ii<(nCell-(RTREE_MINCELLS(pRtree)+1)); ii++){
    RtreeCell *p = &aCell[aOrder[ii]];
    nodeInsertCell(pRtree, pNode, p);
    if( p->iRowid==pCell->iRowid ){
      if( iHeight==0 ){
        rc = rowidWrite(pRtree, p->iRowid, pNode->iNode);
      }else{
        rc = parentWrite(pRtree, p->iRowid, pNode->iNode);
      }
    }
  }
  if( rc==SQLITE_OK ){
    rc = fixBoundingBox(pRtree, pNode);
  }
  for(; rc==SQLITE_OK && ii<nCell; ii++){
    /* Find a node to store this cell in. pNode->iNode currently contains
    ** the height of the sub-tree headed by the cell.
    */
    RtreeNode *pInsert;
    RtreeCell *p = &aCell[aOrder[ii]];
    rc = ChooseLeaf(pRtree, p, iHeight, &pInsert);
    if( rc==SQLITE_OK ){
      int rc2;
      rc = rtreeInsertCell(pRtree, pInsert, p, iHeight);
      rc2 = nodeRelease(pRtree, pInsert);
      if( rc==SQLITE_OK ){
        rc = rc2;
      }
    }
  }

  sqlite3_free(aCell);
  return rc;
}

/*
** Insert cell pCell into node pNode. Node pNode is the head of a 
** subtree iHeight high (leaf nodes have iHeight==0).
*/
static int rtreeInsertCell(
  Rtree *pRtree,
  RtreeNode *pNode,
  RtreeCell *pCell,
  int iHeight
){
  int rc = SQLITE_OK;
  if( iHeight>0 ){
    RtreeNode *pChild = nodeHashLookup(pRtree, pCell->iRowid);
    if( pChild ){
      nodeRelease(pRtree, pChild->pParent);
      nodeReference(pNode);
      pChild->pParent = pNode;
    }
  }
  if( nodeInsertCell(pRtree, pNode, pCell) ){
    if( iHeight<=pRtree->iReinsertHeight || pNode->iNode==1){
      rc = SplitNode(pRtree, pNode, pCell, iHeight);
    }else{
      pRtree->iReinsertHeight = iHeight;
      rc = Reinsert(pRtree, pNode, pCell, iHeight);
    }
  }else{
    rc = AdjustTree(pRtree, pNode, pCell);
    if( rc==SQLITE_OK ){
      if( iHeight==0 ){
        rc = rowidWrite(pRtree, pCell->iRowid, pNode->iNode);
      }else{
        rc = parentWrite(pRtree, pCell->iRowid, pNode->iNode);
      }
    }
  }
  return rc;
}

static int reinsertNodeContent(Rtree *pRtree, RtreeNode *pNode){
  int ii;
  int rc = SQLITE_OK;
  int nCell = NCELL(pNode);

  for(ii=0; rc==SQLITE_OK && ii<nCell; ii++){
    RtreeNode *pInsert;
    RtreeCell cell;
    nodeGetCell(pRtree, pNode, ii, &cell);

    /* Find a node to store this cell in. pNode->iNode currently contains
    ** the height of the sub-tree headed by the cell.
    */
    rc = ChooseLeaf(pRtree, &cell, (int)pNode->iNode, &pInsert);
    if( rc==SQLITE_OK ){
      int rc2;
      rc = rtreeInsertCell(pRtree, pInsert, &cell, (int)pNode->iNode);
      rc2 = nodeRelease(pRtree, pInsert);
      if( rc==SQLITE_OK ){
        rc = rc2;
      }
    }
  }
  return rc;
}

/*
** Select a currently unused rowid for a new r-tree record.
*/
static int newRowid(Rtree *pRtree, i64 *piRowid){
  int rc;
  sqlite3_bind_null(pRtree->pWriteRowid, 1);
  sqlite3_bind_null(pRtree->pWriteRowid, 2);
  sqlite3_step(pRtree->pWriteRowid);
  rc = sqlite3_reset(pRtree->pWriteRowid);
  *piRowid = sqlite3_last_insert_rowid(pRtree->db);
  return rc;
}

/*
** Remove the entry with rowid=iDelete from the r-tree structure.
*/
static int rtreeDeleteRowid(Rtree *pRtree, sqlite3_int64 iDelete){
  int rc;                         /* Return code */
  RtreeNode *pLeaf = 0;           /* Leaf node containing record iDelete */
  int iCell;                      /* Index of iDelete cell in pLeaf */
  RtreeNode *pRoot;               /* Root node of rtree structure */


  /* Obtain a reference to the root node to initialize Rtree.iDepth */
  rc = nodeAcquire(pRtree, 1, 0, &pRoot);

  /* Obtain a reference to the leaf node that contains the entry 
  ** about to be deleted. 
  */
  if( rc==SQLITE_OK ){
    rc = findLeafNode(pRtree, iDelete, &pLeaf, 0);
  }

  /* Delete the cell in question from the leaf node. */
  if( rc==SQLITE_OK ){
    int rc2;
    rc = nodeRowidIndex(pRtree, pLeaf, iDelete, &iCell);
    if( rc==SQLITE_OK ){
      rc = deleteCell(pRtree, pLeaf, iCell, 0);
    }
    rc2 = nodeRelease(pRtree, pLeaf);
    if( rc==SQLITE_OK ){
      rc = rc2;
    }
  }

  /* Delete the corresponding entry in the <rtree>_rowid table. */
  if( rc==SQLITE_OK ){
    sqlite3_bind_int64(pRtree->pDeleteRowid, 1, iDelete);
    sqlite3_step(pRtree->pDeleteRowid);
    rc = sqlite3_reset(pRtree->pDeleteRowid);
  }

  /* Check if the root node now has exactly one child. If so, remove
  ** it, schedule the contents of the child for reinsertion and 
  ** reduce the tree height by one.
  **
  ** This is equivalent to copying the contents of the child into
  ** the root node (the operation that Gutman's paper says to perform 
  ** in this scenario).
  */
  if( rc==SQLITE_OK && pRtree->iDepth>0 && NCELL(pRoot)==1 ){
    int rc2;
    RtreeNode *pChild;
    i64 iChild = nodeGetRowid(pRtree, pRoot, 0);
    rc = nodeAcquire(pRtree, iChild, pRoot, &pChild);
    if( rc==SQLITE_OK ){
      rc = removeNode(pRtree, pChild, pRtree->iDepth-1);
    }
    rc2 = nodeRelease(pRtree, pChild);
    if( rc==SQLITE_OK ) rc = rc2;
    if( rc==SQLITE_OK ){
      pRtree->iDepth--;
      writeInt16(pRoot->zData, pRtree->iDepth);
      pRoot->isDirty = 1;
    }
  }

  /* Re-insert the contents of any underfull nodes removed from the tree. */
  for(pLeaf=pRtree->pDeleted; pLeaf; pLeaf=pRtree->pDeleted){
    if( rc==SQLITE_OK ){
      rc = reinsertNodeContent(pRtree, pLeaf);
    }
    pRtree->pDeleted = pLeaf->pNext;
    sqlite3_free(pLeaf);
  }

  /* Release the reference to the root node. */
  if( rc==SQLITE_OK ){
    rc = nodeRelease(pRtree, pRoot);
  }else{
    nodeRelease(pRtree, pRoot);
  }

  return rc;
}

/*
** Rounding constants for float->double conversion.
*/
#define RNDTOWARDS  (1.0 - 1.0/8388608.0)  /* Round towards zero */
#define RNDAWAY     (1.0 + 1.0/8388608.0)  /* Round away from zero */

#if !defined(SQLITE_RTREE_INT_ONLY)
/*
** Convert an sqlite3_value into an RtreeValue (presumably a float)
** while taking care to round toward negative or positive, respectively.
*/
static RtreeValue rtreeValueDown(sqlite3_value *v){
  double d = sqlite3_value_double(v);
  float f = (float)d;
  if( f>d ){
    f = (float)(d*(d<0 ? RNDAWAY : RNDTOWARDS));
  }
  return f;
}
static RtreeValue rtreeValueUp(sqlite3_value *v){
  double d = sqlite3_value_double(v);
  float f = (float)d;
  if( f<d ){
    f = (float)(d*(d<0 ? RNDTOWARDS : RNDAWAY));
  }
  return f;
}
#endif /* !defined(SQLITE_RTREE_INT_ONLY) */


/*
** The xUpdate method for rtree module virtual tables.
*/
static int rtreeUpdate(
  sqlite3_vtab *pVtab, 
  int nData, 
  sqlite3_value **azData, 
  sqlite_int64 *pRowid
){
  Rtree *pRtree = (Rtree *)pVtab;
  int rc = SQLITE_OK;
  RtreeCell cell;                 /* New cell to insert if nData>1 */
  int bHaveRowid = 0;             /* Set to 1 after new rowid is determined */

  rtreeReference(pRtree);
  assert(nData>=1);

  /* Constraint handling. A write operation on an r-tree table may return
  ** SQLITE_CONSTRAINT for two reasons:
  **
  **   1. A duplicate rowid value, or
  **   2. The supplied data violates the "x2>=x1" constraint.
  **
  ** In the first case, if the conflict-handling mode is REPLACE, then
  ** the conflicting row can be removed before proceeding. In the second
  ** case, SQLITE_CONSTRAINT must be returned regardless of the
  ** conflict-handling mode specified by the user.
  */
  if( nData>1 ){
    int ii;

    /* Populate the cell.aCoord[] array. The first coordinate is azData[3]. */
    assert( nData==(pRtree->nDim*2 + 3) );
#ifndef SQLITE_RTREE_INT_ONLY
    if( pRtree->eCoordType==RTREE_COORD_REAL32 ){
      for(ii=0; ii<(pRtree->nDim*2); ii+=2){
        cell.aCoord[ii].f = rtreeValueDown(azData[ii+3]);
        cell.aCoord[ii+1].f = rtreeValueUp(azData[ii+4]);
        if( cell.aCoord[ii].f>cell.aCoord[ii+1].f ){
          rc = SQLITE_CONSTRAINT;
          goto constraint;
        }
      }
    }else
#endif
    {
      for(ii=0; ii<(pRtree->nDim*2); ii+=2){
        cell.aCoord[ii].i = sqlite3_value_int(azData[ii+3]);
        cell.aCoord[ii+1].i = sqlite3_value_int(azData[ii+4]);
        if( cell.aCoord[ii].i>cell.aCoord[ii+1].i ){
          rc = SQLITE_CONSTRAINT;
          goto constraint;
        }
      }
    }

    /* If a rowid value was supplied, check if it is already present in 
    ** the table. If so, the constraint has failed. */
    if( sqlite3_value_type(azData[2])!=SQLITE_NULL ){
      cell.iRowid = sqlite3_value_int64(azData[2]);
      if( sqlite3_value_type(azData[0])==SQLITE_NULL
       || sqlite3_value_int64(azData[0])!=cell.iRowid
      ){
        int steprc;
        sqlite3_bind_int64(pRtree->pReadRowid, 1, cell.iRowid);
        steprc = sqlite3_step(pRtree->pReadRowid);
        rc = sqlite3_reset(pRtree->pReadRowid);
        if( SQLITE_ROW==steprc ){
          if( sqlite3_vtab_on_conflict(pRtree->db)==SQLITE_REPLACE ){
            rc = rtreeDeleteRowid(pRtree, cell.iRowid);
          }else{
            rc = SQLITE_CONSTRAINT;
            goto constraint;
          }
        }
      }
      bHaveRowid = 1;
    }
  }

  /* If azData[0] is not an SQL NULL value, it is the rowid of a
  ** record to delete from the r-tree table. The following block does
  ** just that.
  */
  if( sqlite3_value_type(azData[0])!=SQLITE_NULL ){
    rc = rtreeDeleteRowid(pRtree, sqlite3_value_int64(azData[0]));
  }

  /* If the azData[] array contains more than one element, elements
  ** (azData[2]..azData[argc-1]) contain a new record to insert into
  ** the r-tree structure.
  */
  if( rc==SQLITE_OK && nData>1 ){
    /* Insert the new record into the r-tree */
    RtreeNode *pLeaf = 0;

    /* Figure out the rowid of the new row. */
    if( bHaveRowid==0 ){
      rc = newRowid(pRtree, &cell.iRowid);
    }
    *pRowid = cell.iRowid;

    if( rc==SQLITE_OK ){
      rc = ChooseLeaf(pRtree, &cell, 0, &pLeaf);
    }
    if( rc==SQLITE_OK ){
      int rc2;
      pRtree->iReinsertHeight = -1;
      rc = rtreeInsertCell(pRtree, pLeaf, &cell, 0);
      rc2 = nodeRelease(pRtree, pLeaf);
      if( rc==SQLITE_OK ){
        rc = rc2;
      }
    }
  }

constraint:
  rtreeRelease(pRtree);
  return rc;
}

/*
** The xRename method for rtree module virtual tables.
*/
static int rtreeRename(sqlite3_vtab *pVtab, const char *zNewName){
  Rtree *pRtree = (Rtree *)pVtab;
  int rc = SQLITE_NOMEM;
  char *zSql = sqlite3_mprintf(
    "ALTER TABLE %Q.'%q_node'   RENAME TO \"%w_node\";"
    "ALTER TABLE %Q.'%q_parent' RENAME TO \"%w_parent\";"
    "ALTER TABLE %Q.'%q_rowid'  RENAME TO \"%w_rowid\";"
    , pRtree->zDb, pRtree->zName, zNewName 
    , pRtree->zDb, pRtree->zName, zNewName 
    , pRtree->zDb, pRtree->zName, zNewName
  );
  if( zSql ){
    rc = sqlite3_exec(pRtree->db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }
  return rc;
}

/*
** This function populates the pRtree->nRowEst variable with an estimate
** of the number of rows in the virtual table. If possible, this is based
** on sqlite_stat1 data. Otherwise, use RTREE_DEFAULT_ROWEST.
*/
static int rtreeQueryStat1(sqlite3 *db, Rtree *pRtree){
  const char *zFmt = "SELECT stat FROM %Q.sqlite_stat1 WHERE tbl = '%q_rowid'";
  char *zSql;
  sqlite3_stmt *p;
  int rc;
  i64 nRow = 0;

  zSql = sqlite3_mprintf(zFmt, pRtree->zDb, pRtree->zName);
  if( zSql==0 ){
    rc = SQLITE_NOMEM;
  }else{
    rc = sqlite3_prepare_v2(db, zSql, -1, &p, 0);
    if( rc==SQLITE_OK ){
      if( sqlite3_step(p)==SQLITE_ROW ) nRow = sqlite3_column_int64(p, 0);
      rc = sqlite3_finalize(p);
    }else if( rc!=SQLITE_NOMEM ){
      rc = SQLITE_OK;
    }

    if( rc==SQLITE_OK ){
      if( nRow==0 ){
        pRtree->nRowEst = RTREE_DEFAULT_ROWEST;
      }else{
        pRtree->nRowEst = MAX(nRow, RTREE_MIN_ROWEST);
      }
    }
    sqlite3_free(zSql);
  }

  return rc;
}

static sqlite3_module rtreeModule = {
  0,                          /* iVersion */
  rtreeCreate,                /* xCreate - create a table */
  rtreeConnect,               /* xConnect - connect to an existing table */
  rtreeBestIndex,             /* xBestIndex - Determine search strategy */
  rtreeDisconnect,            /* xDisconnect - Disconnect from a table */
  rtreeDestroy,               /* xDestroy - Drop a table */
  rtreeOpen,                  /* xOpen - open a cursor */
  rtreeClose,                 /* xClose - close a cursor */
  rtreeFilter,                /* xFilter - configure scan constraints */
  rtreeNext,                  /* xNext - advance a cursor */
  rtreeEof,                   /* xEof */
  rtreeColumn,                /* xColumn - read data */
  rtreeRowid,                 /* xRowid - read data */
  rtreeUpdate,                /* xUpdate - write data */
  0,                          /* xBegin - begin transaction */
  0,                          /* xSync - sync transaction */
  0,                          /* xCommit - commit transaction */
  0,                          /* xRollback - rollback transaction */
  0,                          /* xFindFunction - function overloading */
  rtreeRename,                /* xRename - rename the table */
  0,                          /* xSavepoint */
  0,                          /* xRelease */
  0                           /* xRollbackTo */
};

static int rtreeSqlInit(
  Rtree *pRtree, 
  sqlite3 *db, 
  const char *zDb, 
  const char *zPrefix, 
  int isCreate
){
  int rc = SQLITE_OK;

  #define N_STATEMENT 9
  static const char *azSql[N_STATEMENT] = {
    /* Read and write the xxx_node table */
    "SELECT data FROM '%q'.'%q_node' WHERE nodeno = :1",
    "INSERT OR REPLACE INTO '%q'.'%q_node' VALUES(:1, :2)",
    "DELETE FROM '%q'.'%q_node' WHERE nodeno = :1",

    /* Read and write the xxx_rowid table */
    "SELECT nodeno FROM '%q'.'%q_rowid' WHERE rowid = :1",
    "INSERT OR REPLACE INTO '%q'.'%q_rowid' VALUES(:1, :2)",
    "DELETE FROM '%q'.'%q_rowid' WHERE rowid = :1",

    /* Read and write the xxx_parent table */
    "SELECT parentnode FROM '%q'.'%q_parent' WHERE nodeno = :1",
    "INSERT OR REPLACE INTO '%q'.'%q_parent' VALUES(:1, :2)",
    "DELETE FROM '%q'.'%q_parent' WHERE nodeno = :1"
  };
  sqlite3_stmt **appStmt[N_STATEMENT];
  int i;

  pRtree->db = db;

  if( isCreate ){
    char *zCreate = sqlite3_mprintf(
"CREATE TABLE \"%w\".\"%w_node\"(nodeno INTEGER PRIMARY KEY, data BLOB);"
"CREATE TABLE \"%w\".\"%w_rowid\"(rowid INTEGER PRIMARY KEY, nodeno INTEGER);"
"CREATE TABLE \"%w\".\"%w_parent\"(nodeno INTEGER PRIMARY KEY,"
                                  " parentnode INTEGER);"
"INSERT INTO '%q'.'%q_node' VALUES(1, zeroblob(%d))",
      zDb, zPrefix, zDb, zPrefix, zDb, zPrefix, zDb, zPrefix, pRtree->iNodeSize
    );
    if( !zCreate ){
      return SQLITE_NOMEM;
    }
    rc = sqlite3_exec(db, zCreate, 0, 0, 0);
    sqlite3_free(zCreate);
    if( rc!=SQLITE_OK ){
      return rc;
    }
  }

  appStmt[0] = &pRtree->pReadNode;
  appStmt[1] = &pRtree->pWriteNode;
  appStmt[2] = &pRtree->pDeleteNode;
  appStmt[3] = &pRtree->pReadRowid;
  appStmt[4] = &pRtree->pWriteRowid;
  appStmt[5] = &pRtree->pDeleteRowid;
  appStmt[6] = &pRtree->pReadParent;
  appStmt[7] = &pRtree->pWriteParent;
  appStmt[8] = &pRtree->pDeleteParent;

  rc = rtreeQueryStat1(db, pRtree);
  for(i=0; i<N_STATEMENT && rc==SQLITE_OK; i++){
    char *zSql = sqlite3_mprintf(azSql[i], zDb, zPrefix);
    if( zSql ){
      rc = sqlite3_prepare_v2(db, zSql, -1, appStmt[i], 0); 
    }else{
      rc = SQLITE_NOMEM;
    }
    sqlite3_free(zSql);
  }

  return rc;
}

/*
** The second argument to this function contains the text of an SQL statement
** that returns a single integer value. The statement is compiled and executed
** using database connection db. If successful, the integer value returned
** is written to *piVal and SQLITE_OK returned. Otherwise, an SQLite error
** code is returned and the value of *piVal after returning is not defined.
*/
static int getIntFromStmt(sqlite3 *db, const char *zSql, int *piVal){
  int rc = SQLITE_NOMEM;
  if( zSql ){
    sqlite3_stmt *pStmt = 0;
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
    if( rc==SQLITE_OK ){
      if( SQLITE_ROW==sqlite3_step(pStmt) ){
        *piVal = sqlite3_column_int(pStmt, 0);
      }
      rc = sqlite3_finalize(pStmt);
    }
  }
  return rc;
}

/*
** This function is called from within the xConnect() or xCreate() method to
** determine the node-size used by the rtree table being created or connected
** to. If successful, pRtree->iNodeSize is populated and SQLITE_OK returned.
** Otherwise, an SQLite error code is returned.
**
** If this function is being called as part of an xConnect(), then the rtree
** table already exists. In this case the node-size is determined by inspecting
** the root node of the tree.
**
** Otherwise, for an xCreate(), use 64 bytes less than the database page-size. 
** This ensures that each node is stored on a single database page. If the 
** database page-size is so large that more than RTREE_MAXCELLS entries 
** would fit in a single node, use a smaller node-size.
*/
static int getNodeSize(
  sqlite3 *db,                    /* Database handle */
  Rtree *pRtree,                  /* Rtree handle */
  int isCreate,                   /* True for xCreate, false for xConnect */
  char **pzErr                    /* OUT: Error message, if any */
){
  int rc;
  char *zSql;
  if( isCreate ){
    int iPageSize = 0;
    zSql = sqlite3_mprintf("PRAGMA %Q.page_size", pRtree->zDb);
    rc = getIntFromStmt(db, zSql, &iPageSize);
    if( rc==SQLITE_OK ){
      pRtree->iNodeSize = iPageSize-64;
      if( (4+pRtree->nBytesPerCell*RTREE_MAXCELLS)<pRtree->iNodeSize ){
        pRtree->iNodeSize = 4+pRtree->nBytesPerCell*RTREE_MAXCELLS;
      }
    }else{
      *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
    }
  }else{
    zSql = sqlite3_mprintf(
        "SELECT length(data) FROM '%q'.'%q_node' WHERE nodeno = 1",
        pRtree->zDb, pRtree->zName
    );
    rc = getIntFromStmt(db, zSql, &pRtree->iNodeSize);
    if( rc!=SQLITE_OK ){
      *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
    }
  }

  sqlite3_free(zSql);
  return rc;
}

/* 
** This function is the implementation of both the xConnect and xCreate
** methods of the r-tree virtual table.
**
**   argv[0]   -> module name
**   argv[1]   -> database name
**   argv[2]   -> table name
**   argv[...] -> column names...
*/
static int rtreeInit(
  sqlite3 *db,                        /* Database connection */
  void *pAux,                         /* One of the RTREE_COORD_* constants */
  int argc, const char *const*argv,   /* Parameters to CREATE TABLE statement */
  sqlite3_vtab **ppVtab,              /* OUT: New virtual table */
  char **pzErr,                       /* OUT: Error message, if any */
  int isCreate                        /* True for xCreate, false for xConnect */
){
  int rc = SQLITE_OK;
  Rtree *pRtree;
  int nDb;              /* Length of string argv[1] */
  int nName;            /* Length of string argv[2] */
  int eCoordType = (pAux ? RTREE_COORD_INT32 : RTREE_COORD_REAL32);

  const char *aErrMsg[] = {
    0,                                                    /* 0 */
    "Wrong number of columns for an rtree table",         /* 1 */
    "Too few columns for an rtree table",                 /* 2 */
    "Too many columns for an rtree table"                 /* 3 */
  };

  int iErr = (argc<6) ? 2 : argc>(RTREE_MAX_DIMENSIONS*2+4) ? 3 : argc%2;
  if( aErrMsg[iErr] ){
    *pzErr = sqlite3_mprintf("%s", aErrMsg[iErr]);
    return SQLITE_ERROR;
  }

  sqlite3_vtab_config(db, SQLITE_VTAB_CONSTRAINT_SUPPORT, 1);

  /* Allocate the sqlite3_vtab structure */
  nDb = (int)strlen(argv[1]);
  nName = (int)strlen(argv[2]);
  pRtree = (Rtree *)sqlite3_malloc(sizeof(Rtree)+nDb+nName+2);
  if( !pRtree ){
    return SQLITE_NOMEM;
  }
  memset(pRtree, 0, sizeof(Rtree)+nDb+nName+2);
  pRtree->nBusy = 1;
  pRtree->base.pModule = &rtreeModule;
  pRtree->zDb = (char *)&pRtree[1];
  pRtree->zName = &pRtree->zDb[nDb+1];
  pRtree->nDim = (argc-4)/2;
  pRtree->nBytesPerCell = 8 + pRtree->nDim*4*2;
  pRtree->eCoordType = eCoordType;
  memcpy(pRtree->zDb, argv[1], nDb);
  memcpy(pRtree->zName, argv[2], nName);

  /* Figure out the node size to use. */
  rc = getNodeSize(db, pRtree, isCreate, pzErr);

  /* Create/Connect to the underlying relational database schema. If
  ** that is successful, call sqlite3_declare_vtab() to configure
  ** the r-tree table schema.
  */
  if( rc==SQLITE_OK ){
    if( (rc = rtreeSqlInit(pRtree, db, argv[1], argv[2], isCreate)) ){
      *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
    }else{
      char *zSql = sqlite3_mprintf("CREATE TABLE x(%s", argv[3]);
      char *zTmp;
      int ii;
      for(ii=4; zSql && ii<argc; ii++){
        zTmp = zSql;
        zSql = sqlite3_mprintf("%s, %s", zTmp, argv[ii]);
        sqlite3_free(zTmp);
      }
      if( zSql ){
        zTmp = zSql;
        zSql = sqlite3_mprintf("%s);", zTmp);
        sqlite3_free(zTmp);
      }
      if( !zSql ){
        rc = SQLITE_NOMEM;
      }else if( SQLITE_OK!=(rc = sqlite3_declare_vtab(db, zSql)) ){
        *pzErr = sqlite3_mprintf("%s", sqlite3_errmsg(db));
      }
      sqlite3_free(zSql);
    }
  }

  if( rc==SQLITE_OK ){
    *ppVtab = (sqlite3_vtab *)pRtree;
  }else{
    assert( *ppVtab==0 );
    assert( pRtree->nBusy==1 );
    rtreeRelease(pRtree);
  }
  return rc;
}


/*
** Implementation of a scalar function that decodes r-tree nodes to
** human readable strings. This can be used for debugging and analysis.
**
** The scalar function takes two arguments: (1) the number of dimensions
** to the rtree (between 1 and 5, inclusive) and (2) a blob of data containing
** an r-tree node.  For a two-dimensional r-tree structure called "rt", to
** deserialize all nodes, a statement like:
**
**   SELECT rtreenode(2, data) FROM rt_node;
**
** The human readable string takes the form of a Tcl list with one
** entry for each cell in the r-tree node. Each entry is itself a
** list, containing the 8-byte rowid/pageno followed by the 
** <num-dimension>*2 coordinates.
*/
static void rtreenode(sqlite3_context *ctx, int nArg, sqlite3_value **apArg){
  char *zText = 0;
  RtreeNode node;
  Rtree tree;
  int ii;

  UNUSED_PARAMETER(nArg);
  memset(&node, 0, sizeof(RtreeNode));
  memset(&tree, 0, sizeof(Rtree));
  tree.nDim = sqlite3_value_int(apArg[0]);
  tree.nBytesPerCell = 8 + 8 * tree.nDim;
  node.zData = (u8 *)sqlite3_value_blob(apArg[1]);

  for(ii=0; ii<NCELL(&node); ii++){
    char zCell[512];
    int nCell = 0;
    RtreeCell cell;
    int jj;

    nodeGetCell(&tree, &node, ii, &cell);
    sqlite3_snprintf(512-nCell,&zCell[nCell],"%lld", cell.iRowid);
    nCell = (int)strlen(zCell);
    for(jj=0; jj<tree.nDim*2; jj++){
#ifndef SQLITE_RTREE_INT_ONLY
      sqlite3_snprintf(512-nCell,&zCell[nCell], " %g",
                       (double)cell.aCoord[jj].f);
#else
      sqlite3_snprintf(512-nCell,&zCell[nCell], " %d",
                       cell.aCoord[jj].i);
#endif
      nCell = (int)strlen(zCell);
    }

    if( zText ){
      char *zTextNew = sqlite3_mprintf("%s {%s}", zText, zCell);
      sqlite3_free(zText);
      zText = zTextNew;
    }else{
      zText = sqlite3_mprintf("{%s}", zCell);
    }
  }
  
  sqlite3_result_text(ctx, zText, -1, sqlite3_free);
}

/* This routine implements an SQL function that returns the "depth" parameter
** from the front of a blob that is an r-tree node.  For example:
**
**     SELECT rtreedepth(data) FROM rt_node WHERE nodeno=1;
**
** The depth value is 0 for all nodes other than the root node, and the root
** node always has nodeno=1, so the example above is the primary use for this
** routine.  This routine is intended for testing and analysis only.
*/
static void rtreedepth(sqlite3_context *ctx, int nArg, sqlite3_value **apArg){
  UNUSED_PARAMETER(nArg);
  if( sqlite3_value_type(apArg[0])!=SQLITE_BLOB 
   || sqlite3_value_bytes(apArg[0])<2
  ){
    sqlite3_result_error(ctx, "Invalid argument to rtreedepth()", -1); 
  }else{
    u8 *zBlob = (u8 *)sqlite3_value_blob(apArg[0]);
    sqlite3_result_int(ctx, readInt16(zBlob));
  }
}

/*
** Register the r-tree module with database handle db. This creates the
** virtual table module "rtree" and the debugging/analysis scalar 
** function "rtreenode".
*/
SQLITE_PRIVATE int sqlite3RtreeInit(sqlite3 *db){
  const int utf8 = SQLITE_UTF8;
  int rc;

  rc = sqlite3_create_function(db, "rtreenode", 2, utf8, 0, rtreenode, 0, 0);
  if( rc==SQLITE_OK ){
    rc = sqlite3_create_function(db, "rtreedepth", 1, utf8, 0,rtreedepth, 0, 0);
  }
  if( rc==SQLITE_OK ){
#ifdef SQLITE_RTREE_INT_ONLY
    void *c = (void *)RTREE_COORD_INT32;
#else
    void *c = (void *)RTREE_COORD_REAL32;
#endif
    rc = sqlite3_create_module_v2(db, "rtree", &rtreeModule, c, 0);
  }
  if( rc==SQLITE_OK ){
    void *c = (void *)RTREE_COORD_INT32;
    rc = sqlite3_create_module_v2(db, "rtree_i32", &rtreeModule, c, 0);
  }

  return rc;
}

/*
** This routine deletes the RtreeGeomCallback object that was attached
** one of the SQL functions create by sqlite3_rtree_geometry_callback()
** or sqlite3_rtree_query_callback().  In other words, this routine is the
** destructor for an RtreeGeomCallback objecct.  This routine is called when
** the corresponding SQL function is deleted.
*/
static void rtreeFreeCallback(void *p){
  RtreeGeomCallback *pInfo = (RtreeGeomCallback*)p;
  if( pInfo->xDestructor ) pInfo->xDestructor(pInfo->pContext);
  sqlite3_free(p);
}

/*
** Each call to sqlite3_rtree_geometry_callback() or
** sqlite3_rtree_query_callback() creates an ordinary SQLite
** scalar function that is implemented by this routine.
**
** All this function does is construct an RtreeMatchArg object that
** contains the geometry-checking callback routines and a list of
** parameters to this function, then return that RtreeMatchArg object
** as a BLOB.
**
** The R-Tree MATCH operator will read the returned BLOB, deserialize
** the RtreeMatchArg object, and use the RtreeMatchArg object to figure
** out which elements of the R-Tree should be returned by the query.
*/
static void geomCallback(sqlite3_context *ctx, int nArg, sqlite3_value **aArg){
  RtreeGeomCallback *pGeomCtx = (RtreeGeomCallback *)sqlite3_user_data(ctx);
  RtreeMatchArg *pBlob;
  int nBlob;

  nBlob = sizeof(RtreeMatchArg) + (nArg-1)*sizeof(RtreeDValue);
  pBlob = (RtreeMatchArg *)sqlite3_malloc(nBlob);
  if( !pBlob ){
    sqlite3_result_error_nomem(ctx);
  }else{
    int i;
    pBlob->magic = RTREE_GEOMETRY_MAGIC;
    pBlob->cb = pGeomCtx[0];
    pBlob->nParam = nArg;
    for(i=0; i<nArg; i++){
#ifdef SQLITE_RTREE_INT_ONLY
      pBlob->aParam[i] = sqlite3_value_int64(aArg[i]);
#else
      pBlob->aParam[i] = sqlite3_value_double(aArg[i]);
#endif
    }
    sqlite3_result_blob(ctx, pBlob, nBlob, sqlite3_free);
  }
}

/*
** Register a new geometry function for use with the r-tree MATCH operator.
*/
SQLITE_API int sqlite3_rtree_geometry_callback(
  sqlite3 *db,                  /* Register SQL function on this connection */
  const char *zGeom,            /* Name of the new SQL function */
  int (*xGeom)(sqlite3_rtree_geometry*,int,RtreeDValue*,int*), /* Callback */
  void *pContext                /* Extra data associated with the callback */
){
  RtreeGeomCallback *pGeomCtx;      /* Context object for new user-function */

  /* Allocate and populate the context object. */
  pGeomCtx = (RtreeGeomCallback *)sqlite3_malloc(sizeof(RtreeGeomCallback));
  if( !pGeomCtx ) return SQLITE_NOMEM;
  pGeomCtx->xGeom = xGeom;
  pGeomCtx->xQueryFunc = 0;
  pGeomCtx->xDestructor = 0;
  pGeomCtx->pContext = pContext;
  return sqlite3_create_function_v2(db, zGeom, -1, SQLITE_ANY, 
      (void *)pGeomCtx, geomCallback, 0, 0, rtreeFreeCallback
  );
}

/*
** Register a new 2nd-generation geometry function for use with the
** r-tree MATCH operator.
*/
SQLITE_API int sqlite3_rtree_query_callback(
  sqlite3 *db,                 /* Register SQL function on this connection */
  const char *zQueryFunc,      /* Name of new SQL function */
  int (*xQueryFunc)(sqlite3_rtree_query_info*), /* Callback */
  void *pContext,              /* Extra data passed into the callback */
  void (*xDestructor)(void*)   /* Destructor for the extra data */
){
  RtreeGeomCallback *pGeomCtx;      /* Context object for new user-function */

  /* Allocate and populate the context object. */
  pGeomCtx = (RtreeGeomCallback *)sqlite3_malloc(sizeof(RtreeGeomCallback));
  if( !pGeomCtx ) return SQLITE_NOMEM;
  pGeomCtx->xGeom = 0;
  pGeomCtx->xQueryFunc = xQueryFunc;
  pGeomCtx->xDestructor = xDestructor;
  pGeomCtx->pContext = pContext;
  return sqlite3_create_function_v2(db, zQueryFunc, -1, SQLITE_ANY, 
      (void *)pGeomCtx, geomCallback, 0, 0, rtreeFreeCallback
  );
}

#if !SQLITE_CORE
#ifdef _WIN32
__declspec(dllexport)
#endif
SQLITE_API int sqlite3_rtree_init(
  sqlite3 *db,
  char **pzErrMsg,
  const sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi)
  return sqlite3RtreeInit(db);
}
#endif

#endif

/************** End of rtree.c ***********************************************/
/************** Begin file icu.c *********************************************/
/*
** 2007 May 6
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** $Id: icu.c,v 1.7 2007/12/13 21:54:11 drh Exp $
**
** This file implements an integration between the ICU library 
** ("International Components for Unicode", an open-source library 
** for handling unicode data) and SQLite. The integration uses 
** ICU to provide the following to SQLite:
**
**   * An implementation of the SQL regexp() function (and hence REGEXP
**     operator) using the ICU uregex_XX() APIs.
**
**   * Implementations of the SQL scalar upper() and lower() functions
**     for case mapping.
**
**   * Integration of ICU and SQLite collation sequences.
**
**   * An implementation of the LIKE operator that uses ICU to 
**     provide case-independent matching.
*/

#if !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_ICU)

/* Include ICU headers */
#include <unicode/utypes.h>
#include <unicode/uregex.h>
#include <unicode/ustring.h>
#include <unicode/ucol.h>

/* #include <assert.h> */

#ifndef SQLITE_CORE
  SQLITE_EXTENSION_INIT1
#else
#endif

/*
** Maximum length (in bytes) of the pattern in a LIKE or GLOB
** operator.
*/
#ifndef SQLITE_MAX_LIKE_PATTERN_LENGTH
# define SQLITE_MAX_LIKE_PATTERN_LENGTH 50000
#endif

/*
** Version of sqlite3_free() that is always a function, never a macro.
*/
static void xFree(void *p){
  sqlite3_free(p);
}

/*
** Compare two UTF-8 strings for equality where the first string is
** a "LIKE" expression. Return true (1) if they are the same and 
** false (0) if they are different.
*/
static int icuLikeCompare(
  const uint8_t *zPattern,   /* LIKE pattern */
  const uint8_t *zString,    /* The UTF-8 string to compare against */
  const UChar32 uEsc         /* The escape character */
){
  static const int MATCH_ONE = (UChar32)'_';
  static const int MATCH_ALL = (UChar32)'%';

  int iPattern = 0;       /* Current byte index in zPattern */
  int iString = 0;        /* Current byte index in zString */

  int prevEscape = 0;     /* True if the previous character was uEsc */

  while( zPattern[iPattern]!=0 ){

    /* Read (and consume) the next character from the input pattern. */
    UChar32 uPattern;
    U8_NEXT_UNSAFE(zPattern, iPattern, uPattern);
    assert(uPattern!=0);

    /* There are now 4 possibilities:
    **
    **     1. uPattern is an unescaped match-all character "%",
    **     2. uPattern is an unescaped match-one character "_",
    **     3. uPattern is an unescaped escape character, or
    **     4. uPattern is to be handled as an ordinary character
    */
    if( !prevEscape && uPattern==MATCH_ALL ){
      /* Case 1. */
      uint8_t c;

      /* Skip any MATCH_ALL or MATCH_ONE characters that follow a
      ** MATCH_ALL. For each MATCH_ONE, skip one character in the 
      ** test string.
      */
      while( (c=zPattern[iPattern]) == MATCH_ALL || c == MATCH_ONE ){
        if( c==MATCH_ONE ){
          if( zString[iString]==0 ) return 0;
          U8_FWD_1_UNSAFE(zString, iString);
        }
        iPattern++;
      }

      if( zPattern[iPattern]==0 ) return 1;

      while( zString[iString] ){
        if( icuLikeCompare(&zPattern[iPattern], &zString[iString], uEsc) ){
          return 1;
        }
        U8_FWD_1_UNSAFE(zString, iString);
      }
      return 0;

    }else if( !prevEscape && uPattern==MATCH_ONE ){
      /* Case 2. */
      if( zString[iString]==0 ) return 0;
      U8_FWD_1_UNSAFE(zString, iString);

    }else if( !prevEscape && uPattern==uEsc){
      /* Case 3. */
      prevEscape = 1;

    }else{
      /* Case 4. */
      UChar32 uString;
      U8_NEXT_UNSAFE(zString, iString, uString);
      uString = u_foldCase(uString, U_FOLD_CASE_DEFAULT);
      uPattern = u_foldCase(uPattern, U_FOLD_CASE_DEFAULT);
      if( uString!=uPattern ){
        return 0;
      }
      prevEscape = 0;
    }
  }

  return zString[iString]==0;
}

/*
** Implementation of the like() SQL function.  This function implements
** the build-in LIKE operator.  The first argument to the function is the
** pattern and the second argument is the string.  So, the SQL statements:
**
**       A LIKE B
**
** is implemented as like(B, A). If there is an escape character E, 
**
**       A LIKE B ESCAPE E
**
** is mapped to like(B, A, E).
*/
static void icuLikeFunc(
  sqlite3_context *context, 
  int argc, 
  sqlite3_value **argv
){
  const unsigned char *zA = sqlite3_value_text(argv[0]);
  const unsigned char *zB = sqlite3_value_text(argv[1]);
  UChar32 uEsc = 0;

  /* Limit the length of the LIKE or GLOB pattern to avoid problems
  ** of deep recursion and N*N behavior in patternCompare().
  */
  if( sqlite3_value_bytes(argv[0])>SQLITE_MAX_LIKE_PATTERN_LENGTH ){
    sqlite3_result_error(context, "LIKE or GLOB pattern too complex", -1);
    return;
  }


  if( argc==3 ){
    /* The escape character string must consist of a single UTF-8 character.
    ** Otherwise, return an error.
    */
    int nE= sqlite3_value_bytes(argv[2]);
    const unsigned char *zE = sqlite3_value_text(argv[2]);
    int i = 0;
    if( zE==0 ) return;
    U8_NEXT(zE, i, nE, uEsc);
    if( i!=nE){
      sqlite3_result_error(context, 
          "ESCAPE expression must be a single character", -1);
      return;
    }
  }

  if( zA && zB ){
    sqlite3_result_int(context, icuLikeCompare(zA, zB, uEsc));
  }
}

/*
** This function is called when an ICU function called from within
** the implementation of an SQL scalar function returns an error.
**
** The scalar function context passed as the first argument is 
** loaded with an error message based on the following two args.
*/
static void icuFunctionError(
  sqlite3_context *pCtx,       /* SQLite scalar function context */
  const char *zName,           /* Name of ICU function that failed */
  UErrorCode e                 /* Error code returned by ICU function */
){
  char zBuf[128];
  sqlite3_snprintf(128, zBuf, "ICU error: %s(): %s", zName, u_errorName(e));
  zBuf[127] = '\0';
  sqlite3_result_error(pCtx, zBuf, -1);
}

/*
** Function to delete compiled regexp objects. Registered as
** a destructor function with sqlite3_set_auxdata().
*/
static void icuRegexpDelete(void *p){
  URegularExpression *pExpr = (URegularExpression *)p;
  uregex_close(pExpr);
}

/*
** Implementation of SQLite REGEXP operator. This scalar function takes
** two arguments. The first is a regular expression pattern to compile
** the second is a string to match against that pattern. If either 
** argument is an SQL NULL, then NULL Is returned. Otherwise, the result
** is 1 if the string matches the pattern, or 0 otherwise.
**
** SQLite maps the regexp() function to the regexp() operator such
** that the following two are equivalent:
**
**     zString REGEXP zPattern
**     regexp(zPattern, zString)
**
** Uses the following ICU regexp APIs:
**
**     uregex_open()
**     uregex_matches()
**     uregex_close()
*/
static void icuRegexpFunc(sqlite3_context *p, int nArg, sqlite3_value **apArg){
  UErrorCode status = U_ZERO_ERROR;
  URegularExpression *pExpr;
  UBool res;
  const UChar *zString = sqlite3_value_text16(apArg[1]);

  (void)nArg;  /* Unused parameter */

  /* If the left hand side of the regexp operator is NULL, 
  ** then the result is also NULL. 
  */
  if( !zString ){
    return;
  }

  pExpr = sqlite3_get_auxdata(p, 0);
  if( !pExpr ){
    const UChar *zPattern = sqlite3_value_text16(apArg[0]);
    if( !zPattern ){
      return;
    }
    pExpr = uregex_open(zPattern, -1, 0, 0, &status);

    if( U_SUCCESS(status) ){
      sqlite3_set_auxdata(p, 0, pExpr, icuRegexpDelete);
    }else{
      assert(!pExpr);
      icuFunctionError(p, "uregex_open", status);
      return;
    }
  }

  /* Configure the text that the regular expression operates on. */
  uregex_setText(pExpr, zString, -1, &status);
  if( !U_SUCCESS(status) ){
    icuFunctionError(p, "uregex_setText", status);
    return;
  }

  /* Attempt the match */
  res = uregex_matches(pExpr, 0, &status);
  if( !U_SUCCESS(status) ){
    icuFunctionError(p, "uregex_matches", status);
    return;
  }

  /* Set the text that the regular expression operates on to a NULL
  ** pointer. This is not really necessary, but it is tidier than 
  ** leaving the regular expression object configured with an invalid
  ** pointer after this function returns.
  */
  uregex_setText(pExpr, 0, 0, &status);

  /* Return 1 or 0. */
  sqlite3_result_int(p, res ? 1 : 0);
}

/*
** Implementations of scalar functions for case mapping - upper() and 
** lower(). Function upper() converts its input to upper-case (ABC).
** Function lower() converts to lower-case (abc).
**
** ICU provides two types of case mapping, "general" case mapping and
** "language specific". Refer to ICU documentation for the differences
** between the two.
**
** To utilise "general" case mapping, the upper() or lower() scalar 
** functions are invoked with one argument:
**
**     upper('ABC') -> 'abc'
**     lower('abc') -> 'ABC'
**
** To access ICU "language specific" case mapping, upper() or lower()
** should be invoked with two arguments. The second argument is the name
** of the locale to use. Passing an empty string ("") or SQL NULL value
** as the second argument is the same as invoking the 1 argument version
** of upper() or lower().
**
**     lower('I', 'en_us') -> 'i'
**     lower('I', 'tr_tr') -> 'ı' (small dotless i)
**
** http://www.icu-project.org/userguide/posix.html#case_mappings
*/
static void icuCaseFunc16(sqlite3_context *p, int nArg, sqlite3_value **apArg){
  const UChar *zInput;
  UChar *zOutput;
  int nInput;
  int nOutput;

  UErrorCode status = U_ZERO_ERROR;
  const char *zLocale = 0;

  assert(nArg==1 || nArg==2);
  if( nArg==2 ){
    zLocale = (const char *)sqlite3_value_text(apArg[1]);
  }

  zInput = sqlite3_value_text16(apArg[0]);
  if( !zInput ){
    return;
  }
  nInput = sqlite3_value_bytes16(apArg[0]);

  nOutput = nInput * 2 + 2;
  zOutput = sqlite3_malloc(nOutput);
  if( !zOutput ){
    return;
  }

  if( sqlite3_user_data(p) ){
    u_strToUpper(zOutput, nOutput/2, zInput, nInput/2, zLocale, &status);
  }else{
    u_strToLower(zOutput, nOutput/2, zInput, nInput/2, zLocale, &status);
  }

  if( !U_SUCCESS(status) ){
    icuFunctionError(p, "u_strToLower()/u_strToUpper", status);
    return;
  }

  sqlite3_result_text16(p, zOutput, -1, xFree);
}

/*
** Collation sequence destructor function. The pCtx argument points to
** a UCollator structure previously allocated using ucol_open().
*/
static void icuCollationDel(void *pCtx){
  UCollator *p = (UCollator *)pCtx;
  ucol_close(p);
}

/*
** Collation sequence comparison function. The pCtx argument points to
** a UCollator structure previously allocated using ucol_open().
*/
static int icuCollationColl(
  void *pCtx,
  int nLeft,
  const void *zLeft,
  int nRight,
  const void *zRight
){
  UCollationResult res;
  UCollator *p = (UCollator *)pCtx;
  res = ucol_strcoll(p, (UChar *)zLeft, nLeft/2, (UChar *)zRight, nRight/2);
  switch( res ){
    case UCOL_LESS:    return -1;
    case UCOL_GREATER: return +1;
    case UCOL_EQUAL:   return 0;
  }
  assert(!"Unexpected return value from ucol_strcoll()");
  return 0;
}

/*
** Implementation of the scalar function icu_load_collation().
**
** This scalar function is used to add ICU collation based collation 
** types to an SQLite database connection. It is intended to be called
** as follows:
**
**     SELECT icu_load_collation(<locale>, <collation-name>);
**
** Where <locale> is a string containing an ICU locale identifier (i.e.
** "en_AU", "tr_TR" etc.) and <collation-name> is the name of the
** collation sequence to create.
*/
static void icuLoadCollation(
  sqlite3_context *p, 
  int nArg, 
  sqlite3_value **apArg
){
  sqlite3 *db = (sqlite3 *)sqlite3_user_data(p);
  UErrorCode status = U_ZERO_ERROR;
  const char *zLocale;      /* Locale identifier - (eg. "jp_JP") */
  const char *zName;        /* SQL Collation sequence name (eg. "japanese") */
  UCollator *pUCollator;    /* ICU library collation object */
  int rc;                   /* Return code from sqlite3_create_collation_x() */

  assert(nArg==2);
  zLocale = (const char *)sqlite3_value_text(apArg[0]);
  zName = (const char *)sqlite3_value_text(apArg[1]);

  if( !zLocale || !zName ){
    return;
  }

  pUCollator = ucol_open(zLocale, &status);
  if( !U_SUCCESS(status) ){
    icuFunctionError(p, "ucol_open", status);
    return;
  }
  assert(p);

  rc = sqlite3_create_collation_v2(db, zName, SQLITE_UTF16, (void *)pUCollator, 
      icuCollationColl, icuCollationDel
  );
  if( rc!=SQLITE_OK ){
    ucol_close(pUCollator);
    sqlite3_result_error(p, "Error registering collation function", -1);
  }
}

/*
** Register the ICU extension functions with database db.
*/
SQLITE_PRIVATE int sqlite3IcuInit(sqlite3 *db){
  struct IcuScalar {
    const char *zName;                        /* Function name */
    int nArg;                                 /* Number of arguments */
    int enc;                                  /* Optimal text encoding */
    void *pContext;                           /* sqlite3_user_data() context */
    void (*xFunc)(sqlite3_context*,int,sqlite3_value**);
  } scalars[] = {
    {"regexp", 2, SQLITE_ANY,          0, icuRegexpFunc},

    {"lower",  1, SQLITE_UTF16,        0, icuCaseFunc16},
    {"lower",  2, SQLITE_UTF16,        0, icuCaseFunc16},
    {"upper",  1, SQLITE_UTF16, (void*)1, icuCaseFunc16},
    {"upper",  2, SQLITE_UTF16, (void*)1, icuCaseFunc16},

    {"lower",  1, SQLITE_UTF8,         0, icuCaseFunc16},
    {"lower",  2, SQLITE_UTF8,         0, icuCaseFunc16},
    {"upper",  1, SQLITE_UTF8,  (void*)1, icuCaseFunc16},
    {"upper",  2, SQLITE_UTF8,  (void*)1, icuCaseFunc16},

    {"like",   2, SQLITE_UTF8,         0, icuLikeFunc},
    {"like",   3, SQLITE_UTF8,         0, icuLikeFunc},

    {"icu_load_collation",  2, SQLITE_UTF8, (void*)db, icuLoadCollation},
  };

  int rc = SQLITE_OK;
  int i;

  for(i=0; rc==SQLITE_OK && i<(int)(sizeof(scalars)/sizeof(scalars[0])); i++){
    struct IcuScalar *p = &scalars[i];
    rc = sqlite3_create_function(
        db, p->zName, p->nArg, p->enc, p->pContext, p->xFunc, 0, 0
    );
  }

  return rc;
}

#if !SQLITE_CORE
#ifdef _WIN32
__declspec(dllexport)
#endif
SQLITE_API int sqlite3_icu_init(
  sqlite3 *db, 
  char **pzErrMsg,
  const sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi)
  return sqlite3IcuInit(db);
}
#endif

#endif

/************** End of icu.c *************************************************/
/************** Begin file fts3_icu.c ****************************************/
/*
** 2007 June 22
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file implements a tokenizer for fts3 based on the ICU library.
*/
#if !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_FTS3)
#ifdef SQLITE_ENABLE_ICU

/* #include <assert.h> */
/* #include <string.h> */

#include <unicode/ubrk.h>
/* #include <unicode/ucol.h> */
/* #include <unicode/ustring.h> */
#include <unicode/utf16.h>

typedef struct IcuTokenizer IcuTokenizer;
typedef struct IcuCursor IcuCursor;

struct IcuTokenizer {
  sqlite3_tokenizer base;
  char *zLocale;
};

struct IcuCursor {
  sqlite3_tokenizer_cursor base;

  UBreakIterator *pIter;      /* ICU break-iterator object */
  int nChar;                  /* Number of UChar elements in pInput */
  UChar *aChar;               /* Copy of input using utf-16 encoding */
  int *aOffset;               /* Offsets of each character in utf-8 input */

  int nBuffer;
  char *zBuffer;

  int iToken;
};

/*
** Create a new tokenizer instance.
*/
static int icuCreate(
  int argc,                            /* Number of entries in argv[] */
  const char * const *argv,            /* Tokenizer creation arguments */
  sqlite3_tokenizer **ppTokenizer      /* OUT: Created tokenizer */
){
  IcuTokenizer *p;
  int n = 0;

  if( argc>0 ){
    n = strlen(argv[0])+1;
  }
  p = (IcuTokenizer *)sqlite3_malloc(sizeof(IcuTokenizer)+n);
  if( !p ){
    return SQLITE_NOMEM;
  }
  memset(p, 0, sizeof(IcuTokenizer));

  if( n ){
    p->zLocale = (char *)&p[1];
    memcpy(p->zLocale, argv[0], n);
  }

  *ppTokenizer = (sqlite3_tokenizer *)p;

  return SQLITE_OK;
}

/*
** Destroy a tokenizer
*/
static int icuDestroy(sqlite3_tokenizer *pTokenizer){
  IcuTokenizer *p = (IcuTokenizer *)pTokenizer;
  sqlite3_free(p);
  return SQLITE_OK;
}

/*
** Prepare to begin tokenizing a particular string.  The input
** string to be tokenized is pInput[0..nBytes-1].  A cursor
** used to incrementally tokenize this string is returned in 
** *ppCursor.
*/
static int icuOpen(
  sqlite3_tokenizer *pTokenizer,         /* The tokenizer */
  const char *zInput,                    /* Input string */
  int nInput,                            /* Length of zInput in bytes */
  sqlite3_tokenizer_cursor **ppCursor    /* OUT: Tokenization cursor */
){
  IcuTokenizer *p = (IcuTokenizer *)pTokenizer;
  IcuCursor *pCsr;

  const int32_t opt = U_FOLD_CASE_DEFAULT;
  UErrorCode status = U_ZERO_ERROR;
  int nChar;

  UChar32 c;
  int iInput = 0;
  int iOut = 0;

  *ppCursor = 0;

  if( zInput==0 ){
    nInput = 0;
    zInput = "";
  }else if( nInput<0 ){
    nInput = strlen(zInput);
  }
  nChar = nInput+1;
  pCsr = (IcuCursor *)sqlite3_malloc(
      sizeof(IcuCursor) +                /* IcuCursor */
      ((nChar+3)&~3) * sizeof(UChar) +   /* IcuCursor.aChar[] */
      (nChar+1) * sizeof(int)            /* IcuCursor.aOffset[] */
  );
  if( !pCsr ){
    return SQLITE_NOMEM;
  }
  memset(pCsr, 0, sizeof(IcuCursor));
  pCsr->aChar = (UChar *)&pCsr[1];
  pCsr->aOffset = (int *)&pCsr->aChar[(nChar+3)&~3];

  pCsr->aOffset[iOut] = iInput;
  U8_NEXT(zInput, iInput, nInput, c); 
  while( c>0 ){
    int isError = 0;
    c = u_foldCase(c, opt);
    U16_APPEND(pCsr->aChar, iOut, nChar, c, isError);
    if( isError ){
      sqlite3_free(pCsr);
      return SQLITE_ERROR;
    }
    pCsr->aOffset[iOut] = iInput;

    if( iInput<nInput ){
      U8_NEXT(zInput, iInput, nInput, c);
    }else{
      c = 0;
    }
  }

  pCsr->pIter = ubrk_open(UBRK_WORD, p->zLocale, pCsr->aChar, iOut, &status);
  if( !U_SUCCESS(status) ){
    sqlite3_free(pCsr);
    return SQLITE_ERROR;
  }
  pCsr->nChar = iOut;

  ubrk_first(pCsr->pIter);
  *ppCursor = (sqlite3_tokenizer_cursor *)pCsr;
  return SQLITE_OK;
}

/*
** Close a tokenization cursor previously opened by a call to icuOpen().
*/
static int icuClose(sqlite3_tokenizer_cursor *pCursor){
  IcuCursor *pCsr = (IcuCursor *)pCursor;
  ubrk_close(pCsr->pIter);
  sqlite3_free(pCsr->zBuffer);
  sqlite3_free(pCsr);
  return SQLITE_OK;
}

/*
** Extract the next token from a tokenization cursor.
*/
static int icuNext(
  sqlite3_tokenizer_cursor *pCursor,  /* Cursor returned by simpleOpen */
  const char **ppToken,               /* OUT: *ppToken is the token text */
  int *pnBytes,                       /* OUT: Number of bytes in token */
  int *piStartOffset,                 /* OUT: Starting offset of token */
  int *piEndOffset,                   /* OUT: Ending offset of token */
  int *piPosition                     /* OUT: Position integer of token */
){
  IcuCursor *pCsr = (IcuCursor *)pCursor;

  int iStart = 0;
  int iEnd = 0;
  int nByte = 0;

  while( iStart==iEnd ){
    UChar32 c;

    iStart = ubrk_current(pCsr->pIter);
    iEnd = ubrk_next(pCsr->pIter);
    if( iEnd==UBRK_DONE ){
      return SQLITE_DONE;
    }

    while( iStart<iEnd ){
      int iWhite = iStart;
      U16_NEXT(pCsr->aChar, iWhite, pCsr->nChar, c);
      if( u_isspace(c) ){
        iStart = iWhite;
      }else{
        break;
      }
    }
    assert(iStart<=iEnd);
  }

  do {
    UErrorCode status = U_ZERO_ERROR;
    if( nByte ){
      char *zNew = sqlite3_realloc(pCsr->zBuffer, nByte);
      if( !zNew ){
        return SQLITE_NOMEM;
      }
      pCsr->zBuffer = zNew;
      pCsr->nBuffer = nByte;
    }

    u_strToUTF8(
        pCsr->zBuffer, pCsr->nBuffer, &nByte,    /* Output vars */
        &pCsr->aChar[iStart], iEnd-iStart,       /* Input vars */
        &status                                  /* Output success/failure */
    );
  } while( nByte>pCsr->nBuffer );

  *ppToken = pCsr->zBuffer;
  *pnBytes = nByte;
  *piStartOffset = pCsr->aOffset[iStart];
  *piEndOffset = pCsr->aOffset[iEnd];
  *piPosition = pCsr->iToken++;

  return SQLITE_OK;
}

/*
** The set of routines that implement the simple tokenizer
*/
static const sqlite3_tokenizer_module icuTokenizerModule = {
  0,                           /* iVersion */
  icuCreate,                   /* xCreate  */
  icuDestroy,                  /* xCreate  */
  icuOpen,                     /* xOpen    */
  icuClose,                    /* xClose   */
  icuNext,                     /* xNext    */
};

/*
** Set *ppModule to point at the implementation of the ICU tokenizer.
*/
SQLITE_PRIVATE void sqlite3Fts3IcuTokenizerModule(
  sqlite3_tokenizer_module const**ppModule
){
  *ppModule = &icuTokenizerModule;
}

#endif /* defined(SQLITE_ENABLE_ICU) */
#endif /* !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_FTS3) */

/************** End of fts3_icu.c ********************************************/
/************** Begin file sqlite3session.c **********************************/

#if defined(SQLITE_ENABLE_SESSION) && defined(SQLITE_ENABLE_PREUPDATE_HOOK)
/* #include <assert.h> */
/* #include <string.h> */

#ifndef SQLITE_AMALGAMATION
#endif

typedef struct SessionTable SessionTable;
typedef struct SessionChange SessionChange;
typedef struct SessionBuffer SessionBuffer;
typedef struct SessionInput SessionInput;

/*
** Minimum chunk size used by streaming versions of functions.
*/
#ifndef SESSIONS_STRM_CHUNK_SIZE
# ifdef SQLITE_TEST
#   define SESSIONS_STRM_CHUNK_SIZE 64
# else
#   define SESSIONS_STRM_CHUNK_SIZE 1024
# endif
#endif

/*
** Session handle structure.
*/
struct sqlite3_session {
  sqlite3 *db;                    /* Database handle session is attached to */
  char *zDb;                      /* Name of database session is attached to */
  int bEnable;                    /* True if currently recording */
  int bIndirect;                  /* True if all changes are indirect */
  int bAutoAttach;                /* True to auto-attach tables */
  int rc;                         /* Non-zero if an error has occurred */
  void *pFilterCtx;               /* First argument to pass to xTableFilter */
  int (*xTableFilter)(void *pCtx, const char *zTab);
  sqlite3_session *pNext;         /* Next session object on same db. */
  SessionTable *pTable;           /* List of attached tables */
};

/*
** Instances of this structure are used to build strings or binary records.
*/
struct SessionBuffer {
  u8 *aBuf;                       /* Pointer to changeset buffer */
  int nBuf;                       /* Size of buffer aBuf */
  int nAlloc;                     /* Size of allocation containing aBuf */
};

/*
** An object of this type is used internally as an abstraction for 
** input data. Input data may be supplied either as a single large buffer
** (e.g. sqlite3changeset_start()) or using a stream function (e.g.
**  sqlite3changeset_start_strm()).
*/
struct SessionInput {
  int iNext;                      /* Offset in aData[] of next change */
  u8 *aData;                      /* Pointer to buffer containing changeset */
  int nData;                      /* Number of bytes in aData */

  SessionBuffer buf;              /* Current read buffer */
  int (*xInput)(void*, void*, int*);        /* Input stream call (or NULL) */
  void *pIn;                                /* First argument to xInput */
  int bEof;                       /* Set to true after xInput finished */
};

/*
** Structure for changeset iterators.
*/
struct sqlite3_changeset_iter {
  SessionInput in;                /* Input buffer or stream */
  SessionBuffer tblhdr;           /* Buffer to hold apValue/zTab/abPK/ */
  int bPatchset;                  /* True if this is a patchset */
  int rc;                         /* Iterator error code */
  sqlite3_stmt *pConflict;        /* Points to conflicting row, if any */
  char *zTab;                     /* Current table */
  int nCol;                       /* Number of columns in zTab */
  int op;                         /* Current operation */
  int bIndirect;                  /* True if current change was indirect */
  u8 *abPK;                       /* Primary key array */
  sqlite3_value **apValue;        /* old.* and new.* values */
};

/*
** Each session object maintains a set of the following structures, one
** for each table the session object is monitoring. The structures are
** stored in a linked list starting at sqlite3_session.pTable.
**
** The keys of the SessionTable.aChange[] hash table are all rows that have
** been modified in any way since the session object was attached to the
** table.
**
** The data associated with each hash-table entry is a structure containing
** a subset of the initial values that the modified row contained at the
** start of the session. Or no initial values if the row was inserted.
*/
struct SessionTable {
  SessionTable *pNext;
  char *zName;                    /* Local name of table */
  int nCol;                       /* Number of columns in table zName */
  const char **azCol;             /* Column names */
  u8 *abPK;                       /* Array of primary key flags */
  int nEntry;                     /* Total number of entries in hash table */
  int nChange;                    /* Size of apChange[] array */
  SessionChange **apChange;       /* Hash table buckets */
};

/* 
** RECORD FORMAT:
**
** The following record format is similar to (but not compatible with) that 
** used in SQLite database files. This format is used as part of the 
** change-set binary format, and so must be architecture independent.
**
** Unlike the SQLite database record format, each field is self-contained -
** there is no separation of header and data. Each field begins with a
** single byte describing its type, as follows:
**
**       0x00: Undefined value.
**       0x01: Integer value.
**       0x02: Real value.
**       0x03: Text value.
**       0x04: Blob value.
**       0x05: SQL NULL value.
**
** Note that the above match the definitions of SQLITE_INTEGER, SQLITE_TEXT
** and so on in sqlite3.h. For undefined and NULL values, the field consists
** only of the single type byte. For other types of values, the type byte
** is followed by:
**
**   Text values:
**     A varint containing the number of bytes in the value (encoded using
**     UTF-8). Followed by a buffer containing the UTF-8 representation
**     of the text value. There is no nul terminator.
**
**   Blob values:
**     A varint containing the number of bytes in the value, followed by
**     a buffer containing the value itself.
**
**   Integer values:
**     An 8-byte big-endian integer value.
**
**   Real values:
**     An 8-byte big-endian IEEE 754-2008 real value.
**
** Varint values are encoded in the same way as varints in the SQLite 
** record format.
**
** CHANGESET FORMAT:
**
** A changeset is a collection of DELETE, UPDATE and INSERT operations on
** one or more tables. Operations on a single table are grouped together,
** but may occur in any order (i.e. deletes, updates and inserts are all
** mixed together).
**
** Each group of changes begins with a table header:
**
**   1 byte: Constant 0x54 (capital 'T')
**   Varint: Big-endian integer set to the number of columns in the table.
**   nCol bytes: 0x01 for PK columns, 0x00 otherwise.
**   N bytes: Unqualified table name (encoded using UTF-8). Nul-terminated.
**
** Followed by one or more changes to the table.
**
**   1 byte: Either SQLITE_INSERT, UPDATE or DELETE.
**   1 byte: The "indirect-change" flag.
**   old.* record: (delete and update only)
**   new.* record: (insert and update only)
**
** PATCHSET FORMAT:
**
** A patchset is also a collection of changes. It is similar to a changeset,
** but omits those fields that are not useful if no conflict resolution
** is required when applying the changeset.
**
** Each group of changes begins with a table header:
**
**   1 byte: Constant 0x50 (capital 'P')
**   Varint: Big-endian integer set to the number of columns in the table.
**   nCol bytes: 0x01 for PK columns, 0x00 otherwise.
**   N bytes: Unqualified table name (encoded using UTF-8). Nul-terminated.
**
** Followed by one or more changes to the table.
**
**   1 byte: Either SQLITE_INSERT, UPDATE or DELETE.
**   1 byte: The "indirect-change" flag.
**   single record: (PK fields for DELETE, or full record for INSERT/UPDATE).
*/

/*
** For each row modified during a session, there exists a single instance of
** this structure stored in a SessionTable.aChange[] hash table.
*/
struct SessionChange {
  int op;                         /* One of UPDATE, DELETE, INSERT */
  int bIndirect;                  /* True if this change is "indirect" */
  int nRecord;                    /* Number of bytes in buffer aRecord[] */
  u8 *aRecord;                    /* Buffer containing old.* record */
  SessionChange *pNext;           /* For hash-table collisions */
};

/*
** Write a varint with value iVal into the buffer at aBuf. Return the 
** number of bytes written.
*/
static int sessionVarintPut(u8 *aBuf, int iVal){
  return putVarint32(aBuf, iVal);
}

/*
** Return the number of bytes required to store value iVal as a varint.
*/
static int sessionVarintLen(int iVal){
  return sqlite3VarintLen(iVal);
}

/*
** Read a varint value from aBuf[] into *piVal. Return the number of 
** bytes read.
*/
static int sessionVarintGet(u8 *aBuf, int *piVal){
  return getVarint32(aBuf, *piVal);
}

/*
** Read a 64-bit big-endian integer value from buffer aRec[]. Return
** the value read.
*/
static sqlite3_int64 sessionGetI64(u8 *aRec){
  return (((sqlite3_int64)aRec[0]) << 56)
       + (((sqlite3_int64)aRec[1]) << 48)
       + (((sqlite3_int64)aRec[2]) << 40)
       + (((sqlite3_int64)aRec[3]) << 32)
       + (((sqlite3_int64)aRec[4]) << 24)
       + (((sqlite3_int64)aRec[5]) << 16)
       + (((sqlite3_int64)aRec[6]) <<  8)
       + (((sqlite3_int64)aRec[7]) <<  0);
}

/*
** Write a 64-bit big-endian integer value to the buffer aBuf[].
*/
static void sessionPutI64(u8 *aBuf, sqlite3_int64 i){
  aBuf[0] = (i>>56) & 0xFF;
  aBuf[1] = (i>>48) & 0xFF;
  aBuf[2] = (i>>40) & 0xFF;
  aBuf[3] = (i>>32) & 0xFF;
  aBuf[4] = (i>>24) & 0xFF;
  aBuf[5] = (i>>16) & 0xFF;
  aBuf[6] = (i>> 8) & 0xFF;
  aBuf[7] = (i>> 0) & 0xFF;
}

/*
** This function is used to serialize the contents of value pValue (see
** comment titled "RECORD FORMAT" above).
**
** If it is non-NULL, the serialized form of the value is written to 
** buffer aBuf. *pnWrite is set to the number of bytes written before
** returning. Or, if aBuf is NULL, the only thing this function does is
** set *pnWrite.
**
** If no error occurs, SQLITE_OK is returned. Or, if an OOM error occurs
** within a call to sqlite3_value_text() (may fail if the db is utf-16)) 
** SQLITE_NOMEM is returned.
*/
static int sessionSerializeValue(
  u8 *aBuf,                       /* If non-NULL, write serialized value here */
  sqlite3_value *pValue,          /* Value to serialize */
  int *pnWrite                    /* IN/OUT: Increment by bytes written */
){
  int nByte;                      /* Size of serialized value in bytes */

  if( pValue ){
    int eType;                    /* Value type (SQLITE_NULL, TEXT etc.) */
  
    eType = sqlite3_value_type(pValue);
    if( aBuf ) aBuf[0] = eType;
  
    switch( eType ){
      case SQLITE_NULL: 
        nByte = 1;
        break;
  
      case SQLITE_INTEGER: 
      case SQLITE_FLOAT:
        if( aBuf ){
          /* TODO: SQLite does something special to deal with mixed-endian
          ** floating point values (e.g. ARM7). This code probably should
          ** too.  */
          u64 i;
          if( eType==SQLITE_INTEGER ){
            i = (u64)sqlite3_value_int64(pValue);
          }else{
            double r;
            assert( sizeof(double)==8 && sizeof(u64)==8 );
            r = sqlite3_value_double(pValue);
            memcpy(&i, &r, 8);
          }
          sessionPutI64(&aBuf[1], i);
        }
        nByte = 9; 
        break;
  
      default: {
        u8 *z;
        int n;
        int nVarint;
  
        assert( eType==SQLITE_TEXT || eType==SQLITE_BLOB );
        if( eType==SQLITE_TEXT ){
          z = (u8 *)sqlite3_value_text(pValue);
        }else{
          z = (u8 *)sqlite3_value_blob(pValue);
        }
        if( z==0 ) return SQLITE_NOMEM;
        n = sqlite3_value_bytes(pValue);
        nVarint = sessionVarintLen(n);
  
        if( aBuf ){
          sessionVarintPut(&aBuf[1], n);
          memcpy(&aBuf[nVarint + 1], eType==SQLITE_TEXT ? 
              sqlite3_value_text(pValue) : sqlite3_value_blob(pValue), n
          );
        }
  
        nByte = 1 + nVarint + n;
        break;
      }
    }
  }else{
    nByte = 1;
    if( aBuf ) aBuf[0] = '\0';
  }

  if( pnWrite ) *pnWrite += nByte;
  return SQLITE_OK;
}


/*
** This macro is used to calculate hash key values for data structures. In
** order to use this macro, the entire data structure must be represented
** as a series of unsigned integers. In order to calculate a hash-key value
** for a data structure represented as three such integers, the macro may
** then be used as follows:
**
**    int hash_key_value;
**    hash_key_value = HASH_APPEND(0, <value 1>);
**    hash_key_value = HASH_APPEND(hash_key_value, <value 2>);
**    hash_key_value = HASH_APPEND(hash_key_value, <value 3>);
**
** In practice, the data structures this macro is used for are the primary
** key values of modified rows.
*/
#define HASH_APPEND(hash, add) ((hash) << 3) ^ (hash) ^ (unsigned int)(add)

/*
** Append the hash of the 64-bit integer passed as the second argument to the
** hash-key value passed as the first. Return the new hash-key value.
*/
static unsigned int sessionHashAppendI64(unsigned int h, i64 i){
  h = HASH_APPEND(h, i & 0xFFFFFFFF);
  return HASH_APPEND(h, (i>>32)&0xFFFFFFFF);
}

/*
** Append the hash of the blob passed via the second and third arguments to 
** the hash-key value passed as the first. Return the new hash-key value.
*/
static unsigned int sessionHashAppendBlob(unsigned int h, int n, const u8 *z){
  int i;
  for(i=0; i<n; i++) h = HASH_APPEND(h, z[i]);
  return h;
}

/*
** Append the hash of the data type passed as the second argument to the
** hash-key value passed as the first. Return the new hash-key value.
*/
static unsigned int sessionHashAppendType(unsigned int h, int eType){
  return HASH_APPEND(h, eType);
}

/*
** This function may only be called from within a pre-update callback.
** It calculates a hash based on the primary key values of the old.* or 
** new.* row currently available and, assuming no error occurs, writes it to
** *piHash before returning. If the primary key contains one or more NULL
** values, *pbNullPK is set to true before returning.
**
** If an error occurs, an SQLite error code is returned and the final values
** of *piHash asn *pbNullPK are undefined. Otherwise, SQLITE_OK is returned
** and the output variables are set as described above.
*/
static int sessionPreupdateHash(
  sqlite3 *db,                    /* Database handle */
  SessionTable *pTab,             /* Session table handle */
  int bNew,                       /* True to hash the new.* PK */
  int *piHash,                    /* OUT: Hash value */
  int *pbNullPK                   /* OUT: True if there are NULL values in PK */
){
  unsigned int h = 0;             /* Hash value to return */
  int i;                          /* Used to iterate through columns */

  assert( *pbNullPK==0 );
  assert( pTab->nCol==sqlite3_preupdate_count(db) );
  for(i=0; i<pTab->nCol; i++){
    if( pTab->abPK[i] ){
      int rc;
      int eType;
      sqlite3_value *pVal;

      if( bNew ){
        rc = sqlite3_preupdate_new(db, i, &pVal);
      }else{
        rc = sqlite3_preupdate_old(db, i, &pVal);
      }
      if( rc!=SQLITE_OK ) return rc;

      eType = sqlite3_value_type(pVal);
      h = sessionHashAppendType(h, eType);
      if( eType==SQLITE_INTEGER || eType==SQLITE_FLOAT ){
        i64 iVal;
        if( eType==SQLITE_INTEGER ){
          iVal = sqlite3_value_int64(pVal);
        }else{
          double rVal = sqlite3_value_double(pVal);
          assert( sizeof(iVal)==8 && sizeof(rVal)==8 );
          memcpy(&iVal, &rVal, 8);
        }
        h = sessionHashAppendI64(h, iVal);
      }else if( eType==SQLITE_TEXT || eType==SQLITE_BLOB ){
        const u8 *z;
        if( eType==SQLITE_TEXT ){
          z = (const u8 *)sqlite3_value_text(pVal);
        }else{
          z = (const u8 *)sqlite3_value_blob(pVal);
        }
        if( !z ) return SQLITE_NOMEM;
        h = sessionHashAppendBlob(h, sqlite3_value_bytes(pVal), z);
      }else{
        assert( eType==SQLITE_NULL );
        *pbNullPK = 1;
      }
    }
  }

  *piHash = (h % pTab->nChange);
  return SQLITE_OK;
}

/*
** The buffer that the argument points to contains a serialized SQL value.
** Return the number of bytes of space occupied by the value (including
** the type byte).
*/
static int sessionSerialLen(u8 *a){
  int e = *a;
  int n;
  if( e==0 ) return 1;
  if( e==SQLITE_NULL ) return 1;
  if( e==SQLITE_INTEGER || e==SQLITE_FLOAT ) return 9;
  return sessionVarintGet(&a[1], &n) + 1 + n;
}

/*
** Based on the primary key values stored in change aRecord, calculate a
** hash key. Assume the has table has nBucket buckets. The hash keys
** calculated by this function are compatible with those calculated by
** sessionPreupdateHash().
**
** The bPkOnly argument is non-zero if the record at aRecord[] is from
** a patchset DELETE. In this case the non-PK fields are omitted entirely.
*/
static unsigned int sessionChangeHash(
  SessionTable *pTab,             /* Table handle */
  int bPkOnly,                    /* Record consists of PK fields only */
  u8 *aRecord,                    /* Change record */
  int nBucket                     /* Assume this many buckets in hash table */
){
  unsigned int h = 0;             /* Value to return */
  int i;                          /* Used to iterate through columns */
  u8 *a = aRecord;                /* Used to iterate through change record */

  for(i=0; i<pTab->nCol; i++){
    int eType = *a;
    int isPK = pTab->abPK[i];
    if( bPkOnly && isPK==0 ) continue;

    /* It is not possible for eType to be SQLITE_NULL here. The session 
    ** module does not record changes for rows with NULL values stored in
    ** primary key columns. */
    assert( eType==SQLITE_INTEGER || eType==SQLITE_FLOAT 
         || eType==SQLITE_TEXT || eType==SQLITE_BLOB 
         || eType==SQLITE_NULL || eType==0 
    );
    assert( !isPK || (eType!=0 && eType!=SQLITE_NULL) );

    if( isPK ){
      a++;
      h = sessionHashAppendType(h, eType);
      if( eType==SQLITE_INTEGER || eType==SQLITE_FLOAT ){
        h = sessionHashAppendI64(h, sessionGetI64(a));
        a += 8;
      }else{
        int n; 
        a += sessionVarintGet(a, &n);
        h = sessionHashAppendBlob(h, n, a);
        a += n;
      }
    }else{
      a += sessionSerialLen(a);
    }
  }
  return (h % nBucket);
}

/*
** Arguments aLeft and aRight are pointers to change records for table pTab.
** This function returns true if the two records apply to the same row (i.e.
** have the same values stored in the primary key columns), or false 
** otherwise.
*/
static int sessionChangeEqual(
  SessionTable *pTab,             /* Table used for PK definition */
  int bLeftPkOnly,                /* True if aLeft[] contains PK fields only */
  u8 *aLeft,                      /* Change record */
  int bRightPkOnly,               /* True if aRight[] contains PK fields only */
  u8 *aRight                      /* Change record */
){
  u8 *a1 = aLeft;                 /* Cursor to iterate through aLeft */
  u8 *a2 = aRight;                /* Cursor to iterate through aRight */
  int iCol;                       /* Used to iterate through table columns */

  for(iCol=0; iCol<pTab->nCol; iCol++){
    int n1 = sessionSerialLen(a1);
    int n2 = sessionSerialLen(a2);

    if( pTab->abPK[iCol] && (n1!=n2 || memcmp(a1, a2, n1)) ){
      return 0;
    }
    if( pTab->abPK[iCol] || bLeftPkOnly==0 ) a1 += n1;
    if( pTab->abPK[iCol] || bRightPkOnly==0 ) a2 += n2;
  }

  return 1;
}

/*
** Arguments aLeft and aRight both point to buffers containing change
** records with nCol columns. This function "merges" the two records into
** a single records which is written to the buffer at *paOut. *paOut is
** then set to point to one byte after the last byte written before 
** returning.
**
** The merging of records is done as follows: For each column, if the 
** aRight record contains a value for the column, copy the value from
** their. Otherwise, if aLeft contains a value, copy it. If neither
** record contains a value for a given column, then neither does the
** output record.
*/
static void sessionMergeRecord(
  u8 **paOut, 
  int nCol,
  u8 *aLeft,
  u8 *aRight
){
  u8 *a1 = aLeft;                 /* Cursor used to iterate through aLeft */
  u8 *a2 = aRight;                /* Cursor used to iterate through aRight */
  u8 *aOut = *paOut;              /* Output cursor */
  int iCol;                       /* Used to iterate from 0 to nCol */

  for(iCol=0; iCol<nCol; iCol++){
    int n1 = sessionSerialLen(a1);
    int n2 = sessionSerialLen(a2);
    if( *a2 ){
      memcpy(aOut, a2, n2);
      aOut += n2;
    }else{
      memcpy(aOut, a1, n1);
      aOut += n1;
    }
    a1 += n1;
    a2 += n2;
  }

  *paOut = aOut;
}

/*
** This is a helper function used by sessionMergeUpdate().
**
** When this function is called, both *paOne and *paTwo point to a value 
** within a change record. Before it returns, both have been advanced so 
** as to point to the next value in the record.
**
** If, when this function is called, *paTwo points to a valid value (i.e.
** *paTwo[0] is not 0x00 - the "no value" placeholder), a copy of the *paTwo
** pointer is returned and *pnVal is set to the number of bytes in the 
** serialized value. Otherwise, a copy of *paOne is returned and *pnVal
** set to the number of bytes in the value at *paOne. If *paOne points
** to the "no value" placeholder, *pnVal is set to 1. In other words:
**
**   if( *paTwo is valid ) return *paTwo;
**   return *paOne;
**
*/
static u8 *sessionMergeValue(
  u8 **paOne,                     /* IN/OUT: Left-hand buffer pointer */
  u8 **paTwo,                     /* IN/OUT: Right-hand buffer pointer */
  int *pnVal                      /* OUT: Bytes in returned value */
){
  u8 *a1 = *paOne;
  u8 *a2 = *paTwo;
  u8 *pRet = 0;
  int n1;

  assert( a1 );
  if( a2 ){
    int n2 = sessionSerialLen(a2);
    if( *a2 ){
      *pnVal = n2;
      pRet = a2;
    }
    *paTwo = &a2[n2];
  }

  n1 = sessionSerialLen(a1);
  if( pRet==0 ){
    *pnVal = n1;
    pRet = a1;
  }
  *paOne = &a1[n1];

  return pRet;
}

/*
** This function is used by changeset_concat() to merge two UPDATE changes
** on the same row.
*/
static int sessionMergeUpdate(
  u8 **paOut,                     /* IN/OUT: Pointer to output buffer */
  SessionTable *pTab,             /* Table change pertains to */
  int bPatchset,                  /* True if records are patchset records */
  u8 *aOldRecord1,                /* old.* record for first change */
  u8 *aOldRecord2,                /* old.* record for second change */
  u8 *aNewRecord1,                /* new.* record for first change */
  u8 *aNewRecord2                 /* new.* record for second change */
){
  u8 *aOld1 = aOldRecord1;
  u8 *aOld2 = aOldRecord2;
  u8 *aNew1 = aNewRecord1;
  u8 *aNew2 = aNewRecord2;

  u8 *aOut = *paOut;
  int i;

  if( bPatchset==0 ){
    int bRequired = 0;

    assert( aOldRecord1 && aNewRecord1 );

    /* Write the old.* vector first. */
    for(i=0; i<pTab->nCol; i++){
      int nOld;
      u8 *aOld;
      int nNew;
      u8 *aNew;

      aOld = sessionMergeValue(&aOld1, &aOld2, &nOld);
      aNew = sessionMergeValue(&aNew1, &aNew2, &nNew);
      if( pTab->abPK[i] || nOld!=nNew || memcmp(aOld, aNew, nNew) ){
        if( pTab->abPK[i]==0 ) bRequired = 1;
        memcpy(aOut, aOld, nOld);
        aOut += nOld;
      }else{
        *(aOut++) = '\0';
      }
    }

    if( !bRequired ) return 0;
  }

  /* Write the new.* vector */
  aOld1 = aOldRecord1;
  aOld2 = aOldRecord2;
  aNew1 = aNewRecord1;
  aNew2 = aNewRecord2;
  for(i=0; i<pTab->nCol; i++){
    int nOld;
    u8 *aOld;
    int nNew;
    u8 *aNew;

    aOld = sessionMergeValue(&aOld1, &aOld2, &nOld);
    aNew = sessionMergeValue(&aNew1, &aNew2, &nNew);
    if( bPatchset==0 
     && (pTab->abPK[i] || (nOld==nNew && 0==memcmp(aOld, aNew, nNew))) 
    ){
      *(aOut++) = '\0';
    }else{
      memcpy(aOut, aNew, nNew);
      aOut += nNew;
    }
  }

  *paOut = aOut;
  return 1;
}

/*
** This function is only called from within a pre-update-hook callback.
** It determines if the current pre-update-hook change affects the same row
** as the change stored in argument pChange. If so, it returns true. Otherwise
** if the pre-update-hook does not affect the same row as pChange, it returns
** false.
*/
static int sessionPreupdateEqual(
  sqlite3 *db,                    /* Database handle */
  SessionTable *pTab,             /* Table associated with change */
  SessionChange *pChange,         /* Change to compare to */
  int op                          /* Current pre-update operation */
){
  int iCol;                       /* Used to iterate through columns */
  u8 *a = pChange->aRecord;       /* Cursor used to scan change record */

  assert( op==SQLITE_INSERT || op==SQLITE_UPDATE || op==SQLITE_DELETE );
  for(iCol=0; iCol<pTab->nCol; iCol++){
    if( !pTab->abPK[iCol] ){
      a += sessionSerialLen(a);
    }else{
      sqlite3_value *pVal;        /* Value returned by preupdate_new/old */
      int rc;                     /* Error code from preupdate_new/old */
      int eType = *a++;           /* Type of value from change record */

      /* The following calls to preupdate_new() and preupdate_old() can not
      ** fail. This is because they cache their return values, and by the
      ** time control flows to here they have already been called once from
      ** within sessionPreupdateHash(). The first two asserts below verify
      ** this (that the method has already been called). */
      if( op==SQLITE_INSERT ){
        assert( db->pPreUpdate->pNewUnpacked || db->pPreUpdate->aNew );
        rc = sqlite3_preupdate_new(db, iCol, &pVal);
      }else{
        assert( db->pPreUpdate->pUnpacked );
        rc = sqlite3_preupdate_old(db, iCol, &pVal);
      }
      assert( rc==SQLITE_OK );
      if( sqlite3_value_type(pVal)!=eType ) return 0;

      /* A SessionChange object never has a NULL value in a PK column */
      assert( eType==SQLITE_INTEGER || eType==SQLITE_FLOAT
           || eType==SQLITE_BLOB    || eType==SQLITE_TEXT
      );

      if( eType==SQLITE_INTEGER || eType==SQLITE_FLOAT ){
        i64 iVal = sessionGetI64(a);
        a += 8;
        if( eType==SQLITE_INTEGER ){
          if( sqlite3_value_int64(pVal)!=iVal ) return 0;
        }else{
          double rVal;
          assert( sizeof(iVal)==8 && sizeof(rVal)==8 );
          memcpy(&rVal, &iVal, 8);
          if( sqlite3_value_double(pVal)!=rVal ) return 0;
        }
      }else{
        int n;
        const u8 *z;
        a += sessionVarintGet(a, &n);
        if( sqlite3_value_bytes(pVal)!=n ) return 0;
        if( eType==SQLITE_TEXT ){
          z = sqlite3_value_text(pVal);
        }else{
          z = sqlite3_value_blob(pVal);
        }
        if( memcmp(a, z, n) ) return 0;
        a += n;
        break;
      }
    }
  }

  return 1;
}

/*
** If required, grow the hash table used to store changes on table pTab 
** (part of the session pSession). If a fatal OOM error occurs, set the
** session object to failed and return SQLITE_ERROR. Otherwise, return
** SQLITE_OK.
**
** It is possible that a non-fatal OOM error occurs in this function. In
** that case the hash-table does not grow, but SQLITE_OK is returned anyway.
** Growing the hash table in this case is a performance optimization only,
** it is not required for correct operation.
*/
static int sessionGrowHash(int bPatchset, SessionTable *pTab){
  if( pTab->nChange==0 || pTab->nEntry>=(pTab->nChange/2) ){
    int i;
    SessionChange **apNew;
    int nNew = (pTab->nChange ? pTab->nChange : 128) * 2;

    apNew = (SessionChange **)sqlite3_malloc(sizeof(SessionChange *) * nNew);
    if( apNew==0 ){
      if( pTab->nChange==0 ){
        return SQLITE_ERROR;
      }
      return SQLITE_OK;
    }
    memset(apNew, 0, sizeof(SessionChange *) * nNew);

    for(i=0; i<pTab->nChange; i++){
      SessionChange *p;
      SessionChange *pNext;
      for(p=pTab->apChange[i]; p; p=pNext){
        int bPkOnly = (p->op==SQLITE_DELETE && bPatchset);
        int iHash = sessionChangeHash(pTab, bPkOnly, p->aRecord, nNew);
        pNext = p->pNext;
        p->pNext = apNew[iHash];
        apNew[iHash] = p;
      }
    }

    sqlite3_free(pTab->apChange);
    pTab->nChange = nNew;
    pTab->apChange = apNew;
  }

  return SQLITE_OK;
}

/*
** This function queries the database for the names of the columns of table
** zThis, in schema zDb. It is expected that the table has nCol columns. If
** not, SQLITE_SCHEMA is returned and none of the output variables are
** populated.
**
** Otherwise, if they are not NULL, variable *pnCol is set to the number
** of columns in the database table and variable *pzTab is set to point to a
** nul-terminated copy of the table name. *pazCol (if not NULL) is set to
** point to an array of pointers to column names. And *pabPK (again, if not
** NULL) is set to point to an array of booleans - true if the corresponding
** column is part of the primary key.
**
** For example, if the table is declared as:
**
**     CREATE TABLE tbl1(w, x, y, z, PRIMARY KEY(w, z));
**
** Then the four output variables are populated as follows:
**
**     *pnCol  = 4
**     *pzTab  = "tbl1"
**     *pazCol = {"w", "x", "y", "z"}
**     *pabPK  = {1, 0, 0, 1}
**
** All returned buffers are part of the same single allocation, which must
** be freed using sqlite3_free() by the caller. If pazCol was not NULL, then
** pointer *pazCol should be freed to release all memory. Otherwise, pointer
** *pabPK. It is illegal for both pazCol and pabPK to be NULL.
*/
static int sessionTableInfo(
  sqlite3 *db,                    /* Database connection */
  const char *zDb,                /* Name of attached database (e.g. "main") */
  const char *zThis,              /* Table name */
  int *pnCol,                     /* OUT: number of columns */
  const char **pzTab,             /* OUT: Copy of zThis */
  const char ***pazCol,           /* OUT: Array of column names for table */
  u8 **pabPK                      /* OUT: Array of booleans - true for PK col */
){
  char *zPragma;
  sqlite3_stmt *pStmt;
  int rc;
  int nByte;
  int nDbCol = 0;
  int nThis;
  int i;
  u8 *pAlloc;
  char **azCol = 0;
  u8 *abPK;

  assert( pazCol && pabPK );

  nThis = sqlite3Strlen30(zThis);
  zPragma = sqlite3_mprintf("PRAGMA '%q'.table_info('%q')", zDb, zThis);
  if( !zPragma ) return SQLITE_NOMEM;

  rc = sqlite3_prepare_v2(db, zPragma, -1, &pStmt, 0);
  sqlite3_free(zPragma);
  if( rc!=SQLITE_OK ) return rc;

  nByte = nThis + 1;
  while( SQLITE_ROW==sqlite3_step(pStmt) ){
    nByte += sqlite3_column_bytes(pStmt, 1);
    nDbCol++;
  }
  rc = sqlite3_reset(pStmt);

  if( rc==SQLITE_OK ){
    nByte += nDbCol * (sizeof(const char *) + sizeof(u8) + 1);
    pAlloc = sqlite3_malloc(nByte);
    if( pAlloc==0 ){
      rc = SQLITE_NOMEM;
    }
  }
  if( rc==SQLITE_OK ){
    azCol = (char **)pAlloc;
    pAlloc = (u8 *)&azCol[nDbCol];
    abPK = (u8 *)pAlloc;
    pAlloc = &abPK[nDbCol];
    if( pzTab ){
      memcpy(pAlloc, zThis, nThis+1);
      *pzTab = (char *)pAlloc;
      pAlloc += nThis+1;
    }
  
    i = 0;
    while( SQLITE_ROW==sqlite3_step(pStmt) ){
      int nName = sqlite3_column_bytes(pStmt, 1);
      const unsigned char *zName = sqlite3_column_text(pStmt, 1);
      if( zName==0 ) break;
      memcpy(pAlloc, zName, nName+1);
      azCol[i] = (char *)pAlloc;
      pAlloc += nName+1;
      abPK[i] = sqlite3_column_int(pStmt, 5);
      i++;
    }
    rc = sqlite3_reset(pStmt);
  
  }

  /* If successful, populate the output variables. Otherwise, zero them and
  ** free any allocation made. An error code will be returned in this case.
  */
  if( rc==SQLITE_OK ){
    *pazCol = (const char **)azCol;
    *pabPK = abPK;
    *pnCol = nDbCol;
  }else{
    *pazCol = 0;
    *pabPK = 0;
    *pnCol = 0;
    if( pzTab ) *pzTab = 0;
    sqlite3_free(azCol);
  }
  sqlite3_finalize(pStmt);
  return rc;
}

/*
** This function is only called from within a pre-update handler for a
** write to table pTab, part of session pSession. If this is the first
** write to this table, set the SessionTable.nCol variable to the number
** of columns in the table.
**
** Otherwise, if this is not the first time this table has been written
** to, check that the number of columns in the table has not changed. If
** it has not, return zero.
**
** If the number of columns in the table has changed since the last write
** was recorded, set the session error-code to SQLITE_SCHEMA and return
** non-zero. Users are not allowed to change the number of columns in a table
** for which changes are being recorded by the session module. If they do so, 
** it is an error.
*/
static int sessionInitTable(sqlite3_session *pSession, SessionTable *pTab){
  if( pTab->nCol==0 ){
    assert( pTab->azCol==0 || pTab->abPK==0 );
    pSession->rc = sessionTableInfo(pSession->db, pSession->zDb, 
        pTab->zName, &pTab->nCol, 0, &pTab->azCol, &pTab->abPK
    );
  }
  if( pSession->rc==SQLITE_OK 
   && pTab->nCol!=sqlite3_preupdate_count(pSession->db) 
  ){
    pSession->rc = SQLITE_SCHEMA;
  }
  return pSession->rc;
}

/*
** This function is only called from with a pre-update-hook reporting a 
** change on table pTab (attached to session pSession). The type of change
** (UPDATE, INSERT, DELETE) is specified by the first argument.
**
** Unless one is already present or an error occurs, an entry is added
** to the changed-rows hash table associated with table pTab.
*/
static void sessionPreupdateOneChange(
  int op,                         /* One of SQLITE_UPDATE, INSERT, DELETE */
  sqlite3_session *pSession,      /* Session object pTab is attached to */
  SessionTable *pTab              /* Table that change applies to */
){
  sqlite3 *db = pSession->db;
  int iHash; 
  int bNullPk = 0; 
  int rc = SQLITE_OK;

  if( pSession->rc ) return;

  /* Load table details if required */
  if( sessionInitTable(pSession, pTab) ) return;

  /* Grow the hash table if required */
  if( sessionGrowHash(0, pTab) ){
    pSession->rc = SQLITE_NOMEM;
    return;
  }

  /* Calculate the hash-key for this change. If the primary key of the row
  ** includes a NULL value, exit early. Such changes are ignored by the
  ** session module. */
  rc = sessionPreupdateHash(db, pTab, op==SQLITE_INSERT, &iHash, &bNullPk);
  if( rc!=SQLITE_OK ) goto error_out;

  if( bNullPk==0 ){
    /* Search the hash table for an existing record for this row. */
    SessionChange *pC;
    for(pC=pTab->apChange[iHash]; pC; pC=pC->pNext){
      if( sessionPreupdateEqual(db, pTab, pC, op) ) break;
    }

    if( pC==0 ){
      /* Create a new change object containing all the old values (if
      ** this is an SQLITE_UPDATE or SQLITE_DELETE), or just the PK
      ** values (if this is an INSERT). */
      SessionChange *pChange; /* New change object */
      int nByte;              /* Number of bytes to allocate */
      int i;                  /* Used to iterate through columns */
  
      assert( rc==SQLITE_OK );
      pTab->nEntry++;
  
      /* Figure out how large an allocation is required */
      nByte = sizeof(SessionChange);
      for(i=0; i<pTab->nCol; i++){
        sqlite3_value *p = 0;
        if( op!=SQLITE_INSERT ){
          TESTONLY(int trc = ) sqlite3_preupdate_old(pSession->db, i, &p);
          assert( trc==SQLITE_OK );
        }else if( pTab->abPK[i] ){
          TESTONLY(int trc = ) sqlite3_preupdate_new(pSession->db, i, &p);
          assert( trc==SQLITE_OK );
        }

        /* This may fail if SQLite value p contains a utf-16 string that must
        ** be converted to utf-8 and an OOM error occurs while doing so. */
        rc = sessionSerializeValue(0, p, &nByte);
        if( rc!=SQLITE_OK ) goto error_out;
      }
  
      /* Allocate the change object */
      pChange = (SessionChange *)sqlite3_malloc(nByte);
      if( !pChange ){
        rc = SQLITE_NOMEM;
        goto error_out;
      }else{
        memset(pChange, 0, sizeof(SessionChange));
        pChange->aRecord = (u8 *)&pChange[1];
      }
  
      /* Populate the change object. None of the preupdate_old(),
      ** preupdate_new() or SerializeValue() calls below may fail as all
      ** required values and encodings have already been cached in memory.
      ** It is not possible for an OOM to occur in this block. */
      nByte = 0;
      for(i=0; i<pTab->nCol; i++){
        sqlite3_value *p = 0;
        if( op!=SQLITE_INSERT ){
          sqlite3_preupdate_old(pSession->db, i, &p);
        }else if( pTab->abPK[i] ){
          sqlite3_preupdate_new(pSession->db, i, &p);
        }
        sessionSerializeValue(&pChange->aRecord[nByte], p, &nByte);
      }

      /* Add the change to the hash-table */
      if( pSession->bIndirect || sqlite3_preupdate_depth(pSession->db) ){
        pChange->bIndirect = 1;
      }
      pChange->nRecord = nByte;
      pChange->op = op;
      pChange->pNext = pTab->apChange[iHash];
      pTab->apChange[iHash] = pChange;

    }else if( pC->bIndirect ){
      /* If the existing change is considered "indirect", but this current
      ** change is "direct", mark the change object as direct. */
      if( sqlite3_preupdate_depth(pSession->db)==0 && pSession->bIndirect==0 ){
        pC->bIndirect = 0;
      }
    }
  }

  /* If an error has occurred, mark the session object as failed. */
 error_out:
  if( rc!=SQLITE_OK ){
    pSession->rc = rc;
  }
}

/*
** The 'pre-update' hook registered by this module with SQLite databases.
*/
static void xPreUpdate(
  void *pCtx,                     /* Copy of third arg to preupdate_hook() */
  sqlite3 *db,                    /* Database handle */
  int op,                         /* SQLITE_UPDATE, DELETE or INSERT */
  char const *zDb,                /* Database name */
  char const *zName,              /* Table name */
  sqlite3_int64 iKey1,            /* Rowid of row about to be deleted/updated */
  sqlite3_int64 iKey2             /* New rowid value (for a rowid UPDATE) */
){
  sqlite3_session *pSession;
  int nDb = sqlite3Strlen30(zDb);
  int nName = sqlite3Strlen30(zName);

  assert( sqlite3_mutex_held(db->mutex) );

  for(pSession=(sqlite3_session *)pCtx; pSession; pSession=pSession->pNext){
    SessionTable *pTab;

    /* If this session is attached to a different database ("main", "temp" 
    ** etc.), or if it is not currently enabled, there is nothing to do. Skip 
    ** to the next session object attached to this database. */
    if( pSession->bEnable==0 ) continue;
    if( pSession->rc ) continue;
    if( sqlite3_strnicmp(zDb, pSession->zDb, nDb+1) ) continue;

    for(pTab=pSession->pTable; pTab || pSession->bAutoAttach; pTab=pTab->pNext){
      if( !pTab ){
        /* This branch is taken if table zName has not yet been attached to
        ** this session and the auto-attach flag is set.  */

        /* If there is a table-filter configured, invoke it. If it returns 0,
        ** this change will not be recorded. Break out of the loop early in
        ** this case.  */
        if( pSession->xTableFilter 
         && pSession->xTableFilter(pSession->pFilterCtx, zName)==0
        ){
          break;
        }

        pSession->rc = sqlite3session_attach(pSession,zName);
        if( pSession->rc ) break;
        pTab = pSession->pTable;
        assert( 0==sqlite3_strnicmp(pTab->zName, zName, nName+1) );
      }

      if( 0==sqlite3_strnicmp(pTab->zName, zName, nName+1) ){
        sessionPreupdateOneChange(op, pSession, pTab);
        if( op==SQLITE_UPDATE ){
          sessionPreupdateOneChange(SQLITE_INSERT, pSession, pTab);
        }
        break;
      }
    }
  }
}

/*
** Create a session object. This session object will record changes to
** database zDb attached to connection db.
*/
SQLITE_API int sqlite3session_create(
  sqlite3 *db,                    /* Database handle */
  const char *zDb,                /* Name of db (e.g. "main") */
  sqlite3_session **ppSession     /* OUT: New session object */
){
  sqlite3_session *pNew;          /* Newly allocated session object */
  sqlite3_session *pOld;          /* Session object already attached to db */
  int nDb = sqlite3Strlen30(zDb); /* Length of zDb in bytes */

  /* Zero the output value in case an error occurs. */
  *ppSession = 0;

  /* Allocate and populate the new session object. */
  pNew = (sqlite3_session *)sqlite3_malloc(sizeof(sqlite3_session) + nDb + 1);
  if( !pNew ) return SQLITE_NOMEM;
  memset(pNew, 0, sizeof(sqlite3_session));
  pNew->db = db;
  pNew->zDb = (char *)&pNew[1];
  pNew->bEnable = 1;
  memcpy(pNew->zDb, zDb, nDb+1);

  /* Add the new session object to the linked list of session objects 
  ** attached to database handle $db. Do this under the cover of the db
  ** handle mutex.  */
  sqlite3_mutex_enter(sqlite3_db_mutex(db));
  pOld = (sqlite3_session*)sqlite3_preupdate_hook(db, xPreUpdate, (void*)pNew);
  pNew->pNext = pOld;
  sqlite3_mutex_leave(sqlite3_db_mutex(db));

  *ppSession = pNew;
  return SQLITE_OK;
}

/*
** Free the list of table objects passed as the first argument. The contents
** of the changed-rows hash tables are also deleted.
*/
static void sessionDeleteTable(SessionTable *pList){
  SessionTable *pNext;
  SessionTable *pTab;

  for(pTab=pList; pTab; pTab=pNext){
    int i;
    pNext = pTab->pNext;
    for(i=0; i<pTab->nChange; i++){
      SessionChange *p;
      SessionChange *pNext;
      for(p=pTab->apChange[i]; p; p=pNext){
        pNext = p->pNext;
        sqlite3_free(p);
      }
    }
    sqlite3_free((char*)pTab->azCol);  /* cast works around VC++ bug */
    sqlite3_free(pTab->apChange);
    sqlite3_free(pTab);
  }
}

/*
** Delete a session object previously allocated using sqlite3session_create().
*/
SQLITE_API void sqlite3session_delete(sqlite3_session *pSession){
  sqlite3 *db = pSession->db;
  sqlite3_session *pHead;
  sqlite3_session **pp;

  /* Unlink the session from the linked list of sessions attached to the
  ** database handle. Hold the db mutex while doing so.  */
  sqlite3_mutex_enter(sqlite3_db_mutex(db));
  pHead = (sqlite3_session*)sqlite3_preupdate_hook(db, 0, 0);
  for(pp=&pHead; (*pp)!=pSession; pp=&((*pp)->pNext));
  *pp = (*pp)->pNext;
  if( pHead ) sqlite3_preupdate_hook(db, xPreUpdate, (void *)pHead);
  sqlite3_mutex_leave(sqlite3_db_mutex(db));

  /* Delete all attached table objects. And the contents of their 
  ** associated hash-tables. */
  sessionDeleteTable(pSession->pTable);

  /* Free the session object itself. */
  sqlite3_free(pSession);
}

/*
** Set a table filter on a Session Object.
*/
SQLITE_API void sqlite3session_table_filter(
  sqlite3_session *pSession, 
  int(*xFilter)(void*, const char*),
  void *pCtx                      /* First argument passed to xFilter */
){
  pSession->bAutoAttach = 1;
  pSession->pFilterCtx = pCtx;
  pSession->xTableFilter = xFilter;
}

/*
** Attach a table to a session. All subsequent changes made to the table
** while the session object is enabled will be recorded.
**
** Only tables that have a PRIMARY KEY defined may be attached. It does
** not matter if the PRIMARY KEY is an "INTEGER PRIMARY KEY" (rowid alias)
** or not.
*/
SQLITE_API int sqlite3session_attach(
  sqlite3_session *pSession,      /* Session object */
  const char *zName               /* Table name */
){
  int rc = SQLITE_OK;
  sqlite3_mutex_enter(sqlite3_db_mutex(pSession->db));

  if( !zName ){
    pSession->bAutoAttach = 1;
  }else{
    SessionTable *pTab;           /* New table object (if required) */
    int nName;                    /* Number of bytes in string zName */

    /* First search for an existing entry. If one is found, this call is
    ** a no-op. Return early. */
    nName = sqlite3Strlen30(zName);
    for(pTab=pSession->pTable; pTab; pTab=pTab->pNext){
      if( 0==sqlite3_strnicmp(pTab->zName, zName, nName+1) ) break;
    }

    if( !pTab ){
      /* Allocate new SessionTable object. */
      pTab = (SessionTable *)sqlite3_malloc(sizeof(SessionTable) + nName + 1);
      if( !pTab ){
        rc = SQLITE_NOMEM;
      }else{
        /* Populate the new SessionTable object and link it into the list. */
        memset(pTab, 0, sizeof(SessionTable));
        pTab->zName = (char *)&pTab[1];
        memcpy(pTab->zName, zName, nName+1);
        pTab->pNext = pSession->pTable;
        pSession->pTable = pTab;
      }
    }
  }

  sqlite3_mutex_leave(sqlite3_db_mutex(pSession->db));
  return rc;
}

/*
** Ensure that there is room in the buffer to append nByte bytes of data.
** If not, use sqlite3_realloc() to grow the buffer so that there is.
**
** If successful, return zero. Otherwise, if an OOM condition is encountered,
** set *pRc to SQLITE_NOMEM and return non-zero.
*/
static int sessionBufferGrow(SessionBuffer *p, int nByte, int *pRc){
  if( *pRc==SQLITE_OK && p->nAlloc-p->nBuf<nByte ){
    u8 *aNew;
    int nNew = p->nAlloc ? p->nAlloc : 128;
    do {
      nNew = nNew*2;
    }while( nNew<(p->nBuf+nByte) );

    aNew = (u8 *)sqlite3_realloc(p->aBuf, nNew);
    if( 0==aNew ){
      *pRc = SQLITE_NOMEM;
    }else{
      p->aBuf = aNew;
      p->nAlloc = nNew;
    }
  }
  return (*pRc!=SQLITE_OK);
}

/*
** Append the value passed as the second argument to the buffer passed
** as the first.
**
** This function is a no-op if *pRc is non-zero when it is called.
** Otherwise, if an error occurs, *pRc is set to an SQLite error code
** before returning.
*/
static void sessionAppendValue(SessionBuffer *p, sqlite3_value *pVal, int *pRc){
  int rc = *pRc;
  if( rc==SQLITE_OK ){
    int nByte = 0;
    rc = sessionSerializeValue(0, pVal, &nByte);
    sessionBufferGrow(p, nByte, &rc);
    if( rc==SQLITE_OK ){
      rc = sessionSerializeValue(&p->aBuf[p->nBuf], pVal, 0);
      p->nBuf += nByte;
    }else{
      *pRc = rc;
    }
  }
}

/*
** This function is a no-op if *pRc is other than SQLITE_OK when it is 
** called. Otherwise, append a single byte to the buffer. 
**
** If an OOM condition is encountered, set *pRc to SQLITE_NOMEM before
** returning.
*/
static void sessionAppendByte(SessionBuffer *p, u8 v, int *pRc){
  if( 0==sessionBufferGrow(p, 1, pRc) ){
    p->aBuf[p->nBuf++] = v;
  }
}

/*
** This function is a no-op if *pRc is other than SQLITE_OK when it is 
** called. Otherwise, append a single varint to the buffer. 
**
** If an OOM condition is encountered, set *pRc to SQLITE_NOMEM before
** returning.
*/
static void sessionAppendVarint(SessionBuffer *p, int v, int *pRc){
  if( 0==sessionBufferGrow(p, 9, pRc) ){
    p->nBuf += sessionVarintPut(&p->aBuf[p->nBuf], v);
  }
}

/*
** This function is a no-op if *pRc is other than SQLITE_OK when it is 
** called. Otherwise, append a blob of data to the buffer. 
**
** If an OOM condition is encountered, set *pRc to SQLITE_NOMEM before
** returning.
*/
static void sessionAppendBlob(
  SessionBuffer *p, 
  const u8 *aBlob, 
  int nBlob, 
  int *pRc
){
  if( 0==sessionBufferGrow(p, nBlob, pRc) ){
    memcpy(&p->aBuf[p->nBuf], aBlob, nBlob);
    p->nBuf += nBlob;
  }
}

/*
** This function is a no-op if *pRc is other than SQLITE_OK when it is 
** called. Otherwise, append a string to the buffer. All bytes in the string
** up to (but not including) the nul-terminator are written to the buffer.
**
** If an OOM condition is encountered, set *pRc to SQLITE_NOMEM before
** returning.
*/
static void sessionAppendStr(
  SessionBuffer *p, 
  const char *zStr, 
  int *pRc
){
  int nStr = sqlite3Strlen30(zStr);
  if( 0==sessionBufferGrow(p, nStr, pRc) ){
    memcpy(&p->aBuf[p->nBuf], zStr, nStr);
    p->nBuf += nStr;
  }
}

/*
** This function is a no-op if *pRc is other than SQLITE_OK when it is 
** called. Otherwise, append the string representation of integer iVal
** to the buffer. No nul-terminator is written.
**
** If an OOM condition is encountered, set *pRc to SQLITE_NOMEM before
** returning.
*/
static void sessionAppendInteger(
  SessionBuffer *p,               /* Buffer to append to */
  int iVal,                       /* Value to write the string rep. of */
  int *pRc                        /* IN/OUT: Error code */
){
  char aBuf[24];
  sqlite3_snprintf(sizeof(aBuf)-1, aBuf, "%d", iVal);
  sessionAppendStr(p, aBuf, pRc);
}

/*
** This function is a no-op if *pRc is other than SQLITE_OK when it is 
** called. Otherwise, append the string zStr enclosed in quotes (") and
** with any embedded quote characters escaped to the buffer. No 
** nul-terminator byte is written.
**
** If an OOM condition is encountered, set *pRc to SQLITE_NOMEM before
** returning.
*/
static void sessionAppendIdent(
  SessionBuffer *p,               /* Buffer to a append to */
  const char *zStr,               /* String to quote, escape and append */
  int *pRc                        /* IN/OUT: Error code */
){
  int nStr = sqlite3Strlen30(zStr)*2 + 2 + 1;
  if( 0==sessionBufferGrow(p, nStr, pRc) ){
    char *zOut = (char *)&p->aBuf[p->nBuf];
    const char *zIn = zStr;
    *zOut++ = '"';
    while( *zIn ){
      if( *zIn=='"' ) *zOut++ = '"';
      *zOut++ = *(zIn++);
    }
    *zOut++ = '"';
    p->nBuf = (int)((u8 *)zOut - p->aBuf);
  }
}

/*
** This function is a no-op if *pRc is other than SQLITE_OK when it is
** called. Otherwse, it appends the serialized version of the value stored
** in column iCol of the row that SQL statement pStmt currently points
** to to the buffer.
*/
static void sessionAppendCol(
  SessionBuffer *p,               /* Buffer to append to */
  sqlite3_stmt *pStmt,            /* Handle pointing to row containing value */
  int iCol,                       /* Column to read value from */
  int *pRc                        /* IN/OUT: Error code */
){
  if( *pRc==SQLITE_OK ){
    int eType = sqlite3_column_type(pStmt, iCol);
    sessionAppendByte(p, (u8)eType, pRc);
    if( eType==SQLITE_INTEGER || eType==SQLITE_FLOAT ){
      sqlite3_int64 i;
      u8 aBuf[8];
      if( eType==SQLITE_INTEGER ){
        i = sqlite3_column_int64(pStmt, iCol);
      }else{
        double r = sqlite3_column_double(pStmt, iCol);
        memcpy(&i, &r, 8);
      }
      sessionPutI64(aBuf, i);
      sessionAppendBlob(p, aBuf, 8, pRc);
    }
    if( eType==SQLITE_BLOB || eType==SQLITE_TEXT ){
      u8 *z;
      if( eType==SQLITE_BLOB ){
        z = (u8 *)sqlite3_column_blob(pStmt, iCol);
      }else{
        z = (u8 *)sqlite3_column_text(pStmt, iCol);
      }
      if( z ){
        int nByte = sqlite3_column_bytes(pStmt, iCol);
        sessionAppendVarint(p, nByte, pRc);
        sessionAppendBlob(p, z, nByte, pRc);
      }else{
        *pRc = SQLITE_NOMEM;
      }
    }
  }
}

/*
**
** This function appends an update change to the buffer (see the comments 
** under "CHANGESET FORMAT" at the top of the file). An update change 
** consists of:
**
**   1 byte:  SQLITE_UPDATE (0x17)
**   n bytes: old.* record (see RECORD FORMAT)
**   m bytes: new.* record (see RECORD FORMAT)
**
** The SessionChange object passed as the third argument contains the
** values that were stored in the row when the session began (the old.*
** values). The statement handle passed as the second argument points
** at the current version of the row (the new.* values).
**
** If all of the old.* values are equal to their corresponding new.* value
** (i.e. nothing has changed), then no data at all is appended to the buffer.
**
** Otherwise, the old.* record contains all primary key values and the 
** original values of any fields that have been modified. The new.* record 
** contains the new values of only those fields that have been modified.
*/ 
static int sessionAppendUpdate(
  SessionBuffer *pBuf,            /* Buffer to append to */
  int bPatchset,                  /* True for "patchset", 0 for "changeset" */
  sqlite3_stmt *pStmt,            /* Statement handle pointing at new row */
  SessionChange *p,               /* Object containing old values */
  u8 *abPK                        /* Boolean array - true for PK columns */
){
  int rc = SQLITE_OK;
  SessionBuffer buf2 = {0,0,0}; /* Buffer to accumulate new.* record in */
  int bNoop = 1;                /* Set to zero if any values are modified */
  int nRewind = pBuf->nBuf;     /* Set to zero if any values are modified */
  int i;                        /* Used to iterate through columns */
  u8 *pCsr = p->aRecord;        /* Used to iterate through old.* values */

  sessionAppendByte(pBuf, SQLITE_UPDATE, &rc);
  sessionAppendByte(pBuf, p->bIndirect, &rc);
  for(i=0; i<sqlite3_column_count(pStmt); i++){
    int bChanged = 0;
    int nAdvance;
    int eType = *pCsr;
    switch( eType ){
      case SQLITE_NULL:
        nAdvance = 1;
        if( sqlite3_column_type(pStmt, i)!=SQLITE_NULL ){
          bChanged = 1;
        }
        break;

      case SQLITE_FLOAT:
      case SQLITE_INTEGER: {
        nAdvance = 9;
        if( eType==sqlite3_column_type(pStmt, i) ){
          sqlite3_int64 iVal = sessionGetI64(&pCsr[1]);
          if( eType==SQLITE_INTEGER ){
            if( iVal==sqlite3_column_int64(pStmt, i) ) break;
          }else{
            double dVal;
            memcpy(&dVal, &iVal, 8);
            if( dVal==sqlite3_column_double(pStmt, i) ) break;
          }
        }
        bChanged = 1;
        break;
      }

      default: {
        int nByte;
        int nHdr = 1 + sessionVarintGet(&pCsr[1], &nByte);
        assert( eType==SQLITE_TEXT || eType==SQLITE_BLOB );
        nAdvance = nHdr + nByte;
        if( eType==sqlite3_column_type(pStmt, i) 
         && nByte==sqlite3_column_bytes(pStmt, i) 
         && 0==memcmp(&pCsr[nHdr], sqlite3_column_blob(pStmt, i), nByte)
        ){
          break;
        }
        bChanged = 1;
      }
    }

    /* If at least one field has been modified, this is not a no-op. */
    if( bChanged ) bNoop = 0;

    /* Add a field to the old.* record. This is omitted if this modules is
    ** currently generating a patchset. */
    if( bPatchset==0 ){
      if( bChanged || abPK[i] ){
        sessionAppendBlob(pBuf, pCsr, nAdvance, &rc);
      }else{
        sessionAppendByte(pBuf, 0, &rc);
      }
    }

    /* Add a field to the new.* record. Or the only record if currently
    ** generating a patchset.  */
    if( bChanged || (bPatchset && abPK[i]) ){
      sessionAppendCol(&buf2, pStmt, i, &rc);
    }else{
      sessionAppendByte(&buf2, 0, &rc);
    }

    pCsr += nAdvance;
  }

  if( bNoop ){
    pBuf->nBuf = nRewind;
  }else{
    sessionAppendBlob(pBuf, buf2.aBuf, buf2.nBuf, &rc);
  }
  sqlite3_free(buf2.aBuf);

  return rc;
}

/*
** Append a DELETE change to the buffer passed as the first argument. Use
** the changeset format if argument bPatchset is zero, or the patchset
** format otherwise.
*/
static int sessionAppendDelete(
  SessionBuffer *pBuf,            /* Buffer to append to */
  int bPatchset,                  /* True for "patchset", 0 for "changeset" */
  SessionChange *p,               /* Object containing old values */
  int nCol,                       /* Number of columns in table */
  u8 *abPK                        /* Boolean array - true for PK columns */
){
  int rc = SQLITE_OK;

  sessionAppendByte(pBuf, SQLITE_DELETE, &rc);
  sessionAppendByte(pBuf, p->bIndirect, &rc);

  if( bPatchset==0 ){
    sessionAppendBlob(pBuf, p->aRecord, p->nRecord, &rc);
  }else{
    int i;
    u8 *a = p->aRecord;
    for(i=0; i<nCol; i++){
      u8 *pStart = a;
      int eType = *a++;

      switch( eType ){
        case 0:
        case SQLITE_NULL:
          assert( abPK[i]==0 );
          break;

        case SQLITE_FLOAT:
        case SQLITE_INTEGER:
          a += 8;
          break;

        default: {
          int n;
          a += sessionVarintGet(a, &n);
          a += n;
          break;
        }
      }
      if( abPK[i] ){
        sessionAppendBlob(pBuf, pStart, (int)(a-pStart), &rc);
      }
    }
    assert( (a - p->aRecord)==p->nRecord );
  }

  return rc;
}

/*
** Formulate and prepare a SELECT statement to retrieve a row from table
** zTab in database zDb based on its primary key. i.e.
**
**   SELECT * FROM zDb.zTab WHERE pk1 = ? AND pk2 = ? AND ...
*/
static int sessionSelectStmt(
  sqlite3 *db,                    /* Database handle */
  const char *zDb,                /* Database name */
  const char *zTab,               /* Table name */
  int nCol,                       /* Number of columns in table */
  const char **azCol,             /* Names of table columns */
  u8 *abPK,                       /* PRIMARY KEY  array */
  sqlite3_stmt **ppStmt           /* OUT: Prepared SELECT statement */
){
  int rc = SQLITE_OK;
  int i;
  const char *zSep = "";
  SessionBuffer buf = {0, 0, 0};

  sessionAppendStr(&buf, "SELECT * FROM ", &rc);
  sessionAppendIdent(&buf, zDb, &rc);
  sessionAppendStr(&buf, ".", &rc);
  sessionAppendIdent(&buf, zTab, &rc);
  sessionAppendStr(&buf, " WHERE ", &rc);
  for(i=0; i<nCol; i++){
    if( abPK[i] ){
      sessionAppendStr(&buf, zSep, &rc);
      sessionAppendIdent(&buf, azCol[i], &rc);
      sessionAppendStr(&buf, " = ?", &rc);
      sessionAppendInteger(&buf, i+1, &rc);
      zSep = " AND ";
    }
  }
  if( rc==SQLITE_OK ){
    rc = sqlite3_prepare_v2(db, (char *)buf.aBuf, buf.nBuf, ppStmt, 0);
  }
  sqlite3_free(buf.aBuf);
  return rc;
}

/*
** Bind the PRIMARY KEY values from the change passed in argument pChange
** to the SELECT statement passed as the first argument. The SELECT statement
** is as prepared by function sessionSelectStmt().
**
** Return SQLITE_OK if all PK values are successfully bound, or an SQLite
** error code (e.g. SQLITE_NOMEM) otherwise.
*/
static int sessionSelectBind(
  sqlite3_stmt *pSelect,          /* SELECT from sessionSelectStmt() */
  int nCol,                       /* Number of columns in table */
  u8 *abPK,                       /* PRIMARY KEY array */
  SessionChange *pChange          /* Change structure */
){
  int i;
  int rc = SQLITE_OK;
  u8 *a = pChange->aRecord;

  for(i=0; i<nCol && rc==SQLITE_OK; i++){
    int eType = *a++;

    switch( eType ){
      case 0:
      case SQLITE_NULL:
        assert( abPK[i]==0 );
        break;

      case SQLITE_INTEGER: {
        if( abPK[i] ){
          i64 iVal = sessionGetI64(a);
          rc = sqlite3_bind_int64(pSelect, i+1, iVal);
        }
        a += 8;
        break;
      }

      case SQLITE_FLOAT: {
        if( abPK[i] ){
          double rVal;
          i64 iVal = sessionGetI64(a);
          memcpy(&rVal, &iVal, 8);
          rc = sqlite3_bind_double(pSelect, i+1, rVal);
        }
        a += 8;
        break;
      }

      case SQLITE_TEXT: {
        int n;
        a += sessionVarintGet(a, &n);
        if( abPK[i] ){
          rc = sqlite3_bind_text(pSelect, i+1, (char *)a, n, SQLITE_TRANSIENT);
        }
        a += n;
        break;
      }

      default: {
        int n;
        assert( eType==SQLITE_BLOB );
        a += sessionVarintGet(a, &n);
        if( abPK[i] ){
          rc = sqlite3_bind_blob(pSelect, i+1, a, n, SQLITE_TRANSIENT);
        }
        a += n;
        break;
      }
    }
  }

  return rc;
}

/*
** This function is a no-op if *pRc is set to other than SQLITE_OK when it
** is called. Otherwise, append a serialized table header (part of the binary 
** changeset format) to buffer *pBuf. If an error occurs, set *pRc to an
** SQLite error code before returning.
*/
static void sessionAppendTableHdr(
  SessionBuffer *pBuf,            /* Append header to this buffer */
  int bPatchset,                  /* Use the patchset format if true */
  SessionTable *pTab,             /* Table object to append header for */
  int *pRc                        /* IN/OUT: Error code */
){
  /* Write a table header */
  sessionAppendByte(pBuf, (bPatchset ? 'P' : 'T'), pRc);
  sessionAppendVarint(pBuf, pTab->nCol, pRc);
  sessionAppendBlob(pBuf, pTab->abPK, pTab->nCol, pRc);
  sessionAppendBlob(pBuf, (u8 *)pTab->zName, (int)strlen(pTab->zName)+1, pRc);
}

/*
** Generate either a changeset (if argument bPatchset is zero) or a patchset
** (if it is non-zero) based on the current contents of the session object
** passed as the first argument.
**
** If no error occurs, SQLITE_OK is returned and the new changeset/patchset
** stored in output variables *pnChangeset and *ppChangeset. Or, if an error
** occurs, an SQLite error code is returned and both output variables set 
** to 0.
*/
int sessionGenerateChangeset(
  sqlite3_session *pSession,      /* Session object */
  int bPatchset,                  /* True for patchset, false for changeset */
  int (*xOutput)(void *pOut, const void *pData, int nData),
  void *pOut,                     /* First argument for xOutput */
  int *pnChangeset,               /* OUT: Size of buffer at *ppChangeset */
  void **ppChangeset              /* OUT: Buffer containing changeset */
){
  sqlite3 *db = pSession->db;     /* Source database handle */
  SessionTable *pTab;             /* Used to iterate through attached tables */
  SessionBuffer buf = {0,0,0};    /* Buffer in which to accumlate changeset */
  int rc;                         /* Return code */

  assert( xOutput==0 || (pnChangeset==0 && ppChangeset==0 ) );

  /* Zero the output variables in case an error occurs. If this session
  ** object is already in the error state (sqlite3_session.rc != SQLITE_OK),
  ** this call will be a no-op.  */
  if( xOutput==0 ){
    *pnChangeset = 0;
    *ppChangeset = 0;
  }

  if( pSession->rc ) return pSession->rc;
  rc = sqlite3_exec(pSession->db, "SAVEPOINT changeset", 0, 0, 0);
  if( rc!=SQLITE_OK ) return rc;

  sqlite3_mutex_enter(sqlite3_db_mutex(db));

  for(pTab=pSession->pTable; rc==SQLITE_OK && pTab; pTab=pTab->pNext){
    if( pTab->nEntry ){
      const char *zName = pTab->zName;
      int nCol;                   /* Number of columns in table */
      u8 *abPK;                   /* Primary key array */
      const char **azCol = 0;     /* Table columns */
      int i;                      /* Used to iterate through hash buckets */
      sqlite3_stmt *pSel = 0;     /* SELECT statement to query table pTab */
      int nRewind = buf.nBuf;     /* Initial size of write buffer */
      int nNoop;                  /* Size of buffer after writing tbl header */

      /* Check the table schema is still Ok. */
      rc = sessionTableInfo(db, pSession->zDb, zName, &nCol, 0, &azCol, &abPK);
      if( !rc && (pTab->nCol!=nCol || memcmp(abPK, pTab->abPK, nCol)) ){
        rc = SQLITE_SCHEMA;
      }

      /* Write a table header */
      sessionAppendTableHdr(&buf, bPatchset, pTab, &rc);

      /* Build and compile a statement to execute: */
      if( rc==SQLITE_OK ){
        rc = sessionSelectStmt(
            db, pSession->zDb, zName, nCol, azCol, abPK, &pSel);
      }

      nNoop = buf.nBuf;
      for(i=0; i<pTab->nChange && rc==SQLITE_OK; i++){
        SessionChange *p;         /* Used to iterate through changes */

        for(p=pTab->apChange[i]; rc==SQLITE_OK && p; p=p->pNext){
          rc = sessionSelectBind(pSel, nCol, abPK, p);
          if( rc!=SQLITE_OK ) continue;
          if( sqlite3_step(pSel)==SQLITE_ROW ){
            if( p->op==SQLITE_INSERT ){
              int iCol;
              sessionAppendByte(&buf, SQLITE_INSERT, &rc);
              sessionAppendByte(&buf, p->bIndirect, &rc);
              for(iCol=0; iCol<nCol; iCol++){
                sessionAppendCol(&buf, pSel, iCol, &rc);
              }
            }else{
              rc = sessionAppendUpdate(&buf, bPatchset, pSel, p, abPK);
            }
          }else if( p->op!=SQLITE_INSERT ){
            rc = sessionAppendDelete(&buf, bPatchset, p, nCol, abPK);
          }
          if( rc==SQLITE_OK ){
            rc = sqlite3_reset(pSel);
          }

          /* If the buffer is now larger than SESSIONS_STRM_CHUNK_SIZE, pass
          ** its contents to the xOutput() callback. */
          if( xOutput 
           && rc==SQLITE_OK 
           && buf.nBuf>nNoop 
           && buf.nBuf>SESSIONS_STRM_CHUNK_SIZE 
          ){
            rc = xOutput(pOut, (void*)buf.aBuf, buf.nBuf);
            nNoop = -1;
            buf.nBuf = 0;
          }

        }
      }

      sqlite3_finalize(pSel);
      if( buf.nBuf==nNoop ){
        buf.nBuf = nRewind;
      }
      sqlite3_free((char*)azCol);  /* cast works around VC++ bug */
    }
  }

  if( rc==SQLITE_OK ){
    if( xOutput==0 ){
      *pnChangeset = buf.nBuf;
      *ppChangeset = buf.aBuf;
      buf.aBuf = 0;
    }else if( buf.nBuf>0 ){
      rc = xOutput(pOut, (void*)buf.aBuf, buf.nBuf);
    }
  }

  sqlite3_free(buf.aBuf);
  sqlite3_exec(db, "RELEASE changeset", 0, 0, 0);
  sqlite3_mutex_leave(sqlite3_db_mutex(db));
  return rc;
}

/*
** Obtain a changeset object containing all changes recorded by the 
** session object passed as the first argument.
**
** It is the responsibility of the caller to eventually free the buffer 
** using sqlite3_free().
*/
SQLITE_API int sqlite3session_changeset(
  sqlite3_session *pSession,      /* Session object */
  int *pnChangeset,               /* OUT: Size of buffer at *ppChangeset */
  void **ppChangeset              /* OUT: Buffer containing changeset */
){
  return sessionGenerateChangeset(pSession, 0, 0, 0, pnChangeset, ppChangeset);
}

/*
** Streaming version of sqlite3session_changeset().
*/
SQLITE_API int sqlite3session_changeset_strm(
  sqlite3_session *pSession,
  int (*xOutput)(void *pOut, const void *pData, int nData),
  void *pOut
){
  return sessionGenerateChangeset(pSession, 0, xOutput, pOut, 0, 0);
}

/*
** Streaming version of sqlite3session_patchset().
*/
SQLITE_API int sqlite3session_patchset_strm(
  sqlite3_session *pSession,
  int (*xOutput)(void *pOut, const void *pData, int nData),
  void *pOut
){
  return sessionGenerateChangeset(pSession, 1, xOutput, pOut, 0, 0);
}

/*
** Obtain a patchset object containing all changes recorded by the 
** session object passed as the first argument.
**
** It is the responsibility of the caller to eventually free the buffer 
** using sqlite3_free().
*/
SQLITE_API int sqlite3session_patchset(
  sqlite3_session *pSession,      /* Session object */
  int *pnPatchset,                /* OUT: Size of buffer at *ppChangeset */
  void **ppPatchset               /* OUT: Buffer containing changeset */
){
  return sessionGenerateChangeset(pSession, 1, 0, 0, pnPatchset, ppPatchset);
}

/*
** Enable or disable the session object passed as the first argument.
*/
SQLITE_API int sqlite3session_enable(sqlite3_session *pSession, int bEnable){
  int ret;
  sqlite3_mutex_enter(sqlite3_db_mutex(pSession->db));
  if( bEnable>=0 ){
    pSession->bEnable = bEnable;
  }
  ret = pSession->bEnable;
  sqlite3_mutex_leave(sqlite3_db_mutex(pSession->db));
  return ret;
}

/*
** Enable or disable the session object passed as the first argument.
*/
SQLITE_API int sqlite3session_indirect(sqlite3_session *pSession, int bIndirect){
  int ret;
  sqlite3_mutex_enter(sqlite3_db_mutex(pSession->db));
  if( bIndirect>=0 ){
    pSession->bIndirect = bIndirect;
  }
  ret = pSession->bIndirect;
  sqlite3_mutex_leave(sqlite3_db_mutex(pSession->db));
  return ret;
}

/*
** Return true if there have been no changes to monitored tables recorded
** by the session object passed as the only argument.
*/
SQLITE_API int sqlite3session_isempty(sqlite3_session *pSession){
  int ret = 0;
  SessionTable *pTab;

  sqlite3_mutex_enter(sqlite3_db_mutex(pSession->db));
  for(pTab=pSession->pTable; pTab && ret==0; pTab=pTab->pNext){
    ret = (pTab->nEntry>0);
  }
  sqlite3_mutex_leave(sqlite3_db_mutex(pSession->db));

  return (ret==0);
}

/*
** Do the work for either sqlite3changeset_start() or start_strm().
*/
int sessionChangesetStart(
  sqlite3_changeset_iter **pp,    /* OUT: Changeset iterator handle */
  int (*xInput)(void *pIn, void *pData, int *pnData),
  void *pIn,
  int nChangeset,                 /* Size of buffer pChangeset in bytes */
  void *pChangeset                /* Pointer to buffer containing changeset */
){
  sqlite3_changeset_iter *pRet;   /* Iterator to return */
  int nByte;                      /* Number of bytes to allocate for iterator */

  assert( xInput==0 || (pChangeset==0 && nChangeset==0) );

  /* Zero the output variable in case an error occurs. */
  *pp = 0;

  /* Allocate and initialize the iterator structure. */
  nByte = sizeof(sqlite3_changeset_iter);
  pRet = (sqlite3_changeset_iter *)sqlite3_malloc(nByte);
  if( !pRet ) return SQLITE_NOMEM;
  memset(pRet, 0, sizeof(sqlite3_changeset_iter));
  pRet->in.aData = (u8 *)pChangeset;
  pRet->in.nData = nChangeset;
  pRet->in.xInput = xInput;
  pRet->in.pIn = pIn;
  pRet->in.iNext = 0;
  pRet->in.bEof = (xInput ? 0 : 1);

  /* Populate the output variable and return success. */
  *pp = pRet;
  return SQLITE_OK;
}

/*
** Create an iterator used to iterate through the contents of a changeset.
*/
SQLITE_API int sqlite3changeset_start(
  sqlite3_changeset_iter **pp,    /* OUT: Changeset iterator handle */
  int nChangeset,                 /* Size of buffer pChangeset in bytes */
  void *pChangeset                /* Pointer to buffer containing changeset */
){
  return sessionChangesetStart(pp, 0, 0, nChangeset, pChangeset);
}

/*
** Streaming version of sqlite3changeset_start().
*/
SQLITE_API int sqlite3changeset_start_strm(
  sqlite3_changeset_iter **pp,    /* OUT: Changeset iterator handle */
  int (*xInput)(void *pIn, void *pData, int *pnData),
  void *pIn
){
  return sessionChangesetStart(pp, xInput, pIn, 0, 0);
}

/*
** Ensure that there are at least nByte bytes available in the buffer. Or,
** if there are not nByte bytes remaining in the input, that all available
** data is in the buffer.
**
** Return an SQLite error code if an error occurs, or SQLITE_OK otherwise.
*/
static int sessionInputBuffer(SessionInput *pIn, int nByte){
  int rc = SQLITE_OK;
  if( pIn->xInput ){
    while( !pIn->bEof && (pIn->iNext+nByte)>=pIn->nData && rc==SQLITE_OK ){
      int nNew = SESSIONS_STRM_CHUNK_SIZE;

      if( pIn->iNext>=SESSIONS_STRM_CHUNK_SIZE ){
        int nMove = pIn->buf.nBuf - pIn->iNext;
        memmove(pIn->buf.aBuf, &pIn->buf.aBuf[pIn->iNext], nMove);
        pIn->buf.nBuf -= pIn->iNext;
        pIn->iNext = 0;
      }

      if( SQLITE_OK==sessionBufferGrow(&pIn->buf, nNew, &rc) ){
        rc = pIn->xInput(pIn->pIn, &pIn->buf.aBuf[pIn->buf.nBuf], &nNew);
        if( nNew==0 ){
          pIn->bEof = 1;
        }else{
          pIn->buf.nBuf += nNew;
        }
      }

      pIn->aData = pIn->buf.aBuf;
      pIn->nData = pIn->buf.nBuf;
    }
  }
  return rc;
}

/*
** When this function is called, *ppRec points to the start of a record
** that contains nCol values. This function advances the pointer *ppRec
** until it points to the byte immediately following that record.
*/
static void sessionSkipRecord(
  u8 **ppRec,                     /* IN/OUT: Record pointer */
  int nCol                        /* Number of values in record */
){
  u8 *aRec = *ppRec;
  int i;
  for(i=0; i<nCol; i++){
    int eType = *aRec++;
    if( eType==SQLITE_TEXT || eType==SQLITE_BLOB ){
      int nByte;
      aRec += sessionVarintGet((u8*)aRec, &nByte);
      aRec += nByte;
    }else if( eType==SQLITE_INTEGER || eType==SQLITE_FLOAT ){
      aRec += 8;
    }
  }

  *ppRec = aRec;
}

/*
** This function sets the value of the sqlite3_value object passed as the
** first argument to a copy of the string or blob held in the aData[] 
** buffer. SQLITE_OK is returned if successful, or SQLITE_NOMEM if an OOM
** error occurs.
*/
static int sessionValueSetStr(
  sqlite3_value *pVal,            /* Set the value of this object */
  u8 *aData,                      /* Buffer containing string or blob data */
  int nData,                      /* Size of buffer aData[] in bytes */
  u8 enc                          /* String encoding (0 for blobs) */
){
  /* In theory this code could just pass SQLITE_TRANSIENT as the final
  ** argument to sqlite3ValueSetStr() and have the copy created 
  ** automatically. But doing so makes it difficult to detect any OOM
  ** error. Hence the code to create the copy externally. */
  u8 *aCopy = sqlite3_malloc(nData);
  if( aCopy==0 ) return SQLITE_NOMEM;
  memcpy(aCopy, aData, nData);
  sqlite3ValueSetStr(pVal, nData, (char*)aCopy, enc, sqlite3_free);
  return SQLITE_OK;
}

/*
** Deserialize a single record from a buffer in memory. See "RECORD FORMAT"
** for details.
**
** When this function is called, *paChange points to the start of the record
** to deserialize. Assuming no error occurs, *paChange is set to point to
** one byte after the end of the same record before this function returns.
** If the argument abPK is NULL, then the record contains nCol values. Or,
** if abPK is other than NULL, then the record contains only the PK fields
** (in other words, it is a patchset DELETE record).
**
** If successful, each element of the apOut[] array (allocated by the caller)
** is set to point to an sqlite3_value object containing the value read
** from the corresponding position in the record. If that value is not
** included in the record (i.e. because the record is part of an UPDATE change
** and the field was not modified), the corresponding element of apOut[] is
** set to NULL.
**
** It is the responsibility of the caller to free all sqlite_value structures
** using sqlite3_free().
**
** If an error occurs, an SQLite error code (e.g. SQLITE_NOMEM) is returned.
** The apOut[] array may have been partially populated in this case.
*/
static int sessionReadRecord(
  SessionInput *pIn,              /* Input data */
  int nCol,                       /* Number of values in record */
  u8 *abPK,                       /* Array of primary key flags, or NULL */
  sqlite3_value **apOut           /* Write values to this array */
){
  int i;                          /* Used to iterate through columns */
  int rc = SQLITE_OK;

  for(i=0; i<nCol && rc==SQLITE_OK; i++){
    int eType = 0;                /* Type of value (SQLITE_NULL, TEXT etc.) */
    if( abPK && abPK[i]==0 ) continue;
    rc = sessionInputBuffer(pIn, 9);
    if( rc==SQLITE_OK ){
      eType = pIn->aData[pIn->iNext++];
    }

    assert( apOut[i]==0 );
    if( eType ){
      apOut[i] = sqlite3ValueNew(0);
      if( !apOut[i] ) rc = SQLITE_NOMEM;
    }

    if( rc==SQLITE_OK ){
      u8 *aVal = &pIn->aData[pIn->iNext];
      if( eType==SQLITE_TEXT || eType==SQLITE_BLOB ){
        int nByte;
        pIn->iNext += sessionVarintGet(aVal, &nByte);
        rc = sessionInputBuffer(pIn, nByte);
        if( rc==SQLITE_OK ){
          u8 enc = (eType==SQLITE_TEXT ? SQLITE_UTF8 : 0);
          rc = sessionValueSetStr(apOut[i],&pIn->aData[pIn->iNext],nByte,enc);
        }
        pIn->iNext += nByte;
      }
      if( eType==SQLITE_INTEGER || eType==SQLITE_FLOAT ){
        sqlite3_int64 v = sessionGetI64(aVal);
        if( eType==SQLITE_INTEGER ){
          sqlite3VdbeMemSetInt64(apOut[i], v);
        }else{
          double d;
          memcpy(&d, &v, 8);
          sqlite3VdbeMemSetDouble(apOut[i], d);
        }
        pIn->iNext += 8;
      }
    }
  }

  return rc;
}

/*
** The input pointer currently points to the second byte of a table-header.
** Specifically, to the following:
**
**   + number of columns in table (varint)
**   + array of PK flags (1 byte per column),
**   + table name (nul terminated).
**
** This function ensures that all of the above is present in the input 
** buffer (i.e. that it can be accessed without any calls to xInput()).
** If successful, SQLITE_OK is returned. Otherwise, an SQLite error code.
** The input pointer is not moved.
*/
static int sessionChangesetBufferTblhdr(SessionInput *pIn, int *pnByte){
  int rc = SQLITE_OK;
  int nCol = 0;
  int nRead = 0;

  rc = sessionInputBuffer(pIn, 9);
  if( rc==SQLITE_OK ){
    nRead += sessionVarintGet(&pIn->aData[pIn->iNext + nRead], &nCol);
    rc = sessionInputBuffer(pIn, nRead+nCol+100);
    nRead += nCol;
  }

  while( rc==SQLITE_OK ){
    while( (pIn->iNext + nRead)<pIn->nData && pIn->aData[pIn->iNext + nRead] ){
      nRead++;
    }
    if( (pIn->iNext + nRead)<pIn->nData ) break;
    rc = sessionInputBuffer(pIn, nRead + 100);
  }
  *pnByte = nRead+1;
  return rc;
}

/*
** The input pointer currently points to the first byte of the first field
** of a record consisting of nCol columns. This function ensures the entire
** record is buffered. It does not move the input pointer.
**
** If successful, SQLITE_OK is returned and *pnByte is set to the size of
** the record in bytes. Otherwise, an SQLite error code is returned. The
** final value of *pnByte is undefined in this case.
*/
static int sessionChangesetBufferRecord(
  SessionInput *pIn,              /* Input data */
  int nCol,                       /* Number of columns in record */
  int *pnByte                     /* OUT: Size of record in bytes */
){
  int rc = SQLITE_OK;
  int nByte = 0;
  int i;
  for(i=0; rc==SQLITE_OK && i<nCol; i++){
    int eType;
    rc = sessionInputBuffer(pIn, nByte + 10);
    if( rc==SQLITE_OK ){
      eType = pIn->aData[pIn->iNext + nByte++];
      if( eType==SQLITE_TEXT || eType==SQLITE_BLOB ){
        int n;
        nByte += sessionVarintGet(&pIn->aData[pIn->iNext+nByte], &n);
        nByte += n;
        rc = sessionInputBuffer(pIn, nByte);
      }else if( eType==SQLITE_INTEGER || eType==SQLITE_FLOAT ){
        nByte += 8;
      }
    }
  }
  *pnByte = nByte;
  return rc;
}

/*
** The input pointer currently points to the second byte of a table-header.
** Specifically, to the following:
**
**   + number of columns in table (varint)
**   + array of PK flags (1 byte per column),
**   + table name (nul terminated).
**
** This function decodes the table-header and populates the p->nCol, 
** p->zTab and p->abPK[] variables accordingly. The p->apValue[] array is 
** also allocated or resized according to the new value of p->nCol. The
** input pointer is left pointing to the byte following the table header.
**
** If successful, SQLITE_OK is returned. Otherwise, an SQLite error code
** is returned and the final values of the various fields enumerated above
** are undefined.
*/
static int sessionChangesetReadTblhdr(sqlite3_changeset_iter *p){
  int rc;
  int nCopy;
  assert( p->rc==SQLITE_OK );

  rc = sessionChangesetBufferTblhdr(&p->in, &nCopy);
  if( rc==SQLITE_OK ){
    int nByte;
    int nVarint;
    nVarint = sessionVarintGet(&p->in.aData[p->in.iNext], &p->nCol);
    nCopy -= nVarint;
    p->in.iNext += nVarint;
    nByte = p->nCol * sizeof(sqlite3_value*) * 2 + nCopy;
    p->tblhdr.nBuf = 0;
    sessionBufferGrow(&p->tblhdr, nByte, &rc);
  }

  if( rc==SQLITE_OK ){
    int iPK = sizeof(sqlite3_value*)*p->nCol*2;
    memset(p->tblhdr.aBuf, 0, iPK);
    memcpy(&p->tblhdr.aBuf[iPK], &p->in.aData[p->in.iNext], nCopy);
    p->in.iNext += nCopy;
  }

  p->apValue = (sqlite3_value**)p->tblhdr.aBuf;
  p->abPK = (u8*)&p->apValue[p->nCol*2];
  p->zTab = (char*)&p->abPK[p->nCol];
  return (p->rc = rc);
}

/*
** Advance the changeset iterator to the next change.
**
** If both paRec and pnRec are NULL, then this function works like the public
** API sqlite3changeset_next(). If SQLITE_ROW is returned, then the
** sqlite3changeset_new() and old() APIs may be used to query for values.
**
** Otherwise, if paRec and pnRec are not NULL, then a pointer to the change
** record is written to *paRec before returning and the number of bytes in
** the record to *pnRec.
**
** Either way, this function returns SQLITE_ROW if the iterator is 
** successfully advanced to the next change in the changeset, an SQLite 
** error code if an error occurs, or SQLITE_DONE if there are no further 
** changes in the changeset.
*/
static int sessionChangesetNext(
  sqlite3_changeset_iter *p,      /* Changeset iterator */
  u8 **paRec,                     /* If non-NULL, store record pointer here */
  int *pnRec                      /* If non-NULL, store size of record here */
){
  int i;
  u8 op;

  assert( (paRec==0 && pnRec==0) || (paRec && pnRec) );

  /* If the iterator is in the error-state, return immediately. */
  if( p->rc!=SQLITE_OK ) return p->rc;

  /* Free the current contents of p->apValue[], if any. */
  if( p->apValue ){
    for(i=0; i<p->nCol*2; i++){
      sqlite3ValueFree(p->apValue[i]);
    }
    memset(p->apValue, 0, sizeof(sqlite3_value*)*p->nCol*2);
  }

  /* Make sure the buffer contains at least 10 bytes of input data, or all
  ** remaining data if there are less than 10 bytes available. This is
  ** sufficient either for the 'T' or 'P' byte and the varint that follows
  ** it, or for the two single byte values otherwise. */
  p->rc = sessionInputBuffer(&p->in, 2);
  if( p->rc!=SQLITE_OK ) return p->rc;

  /* If the iterator is already at the end of the changeset, return DONE. */
  if( p->in.iNext>=p->in.nData ){
    return SQLITE_DONE;
  }

  op = p->in.aData[p->in.iNext++];
  if( op=='T' || op=='P' ){
    p->bPatchset = (op=='P');
    if( sessionChangesetReadTblhdr(p) ) return p->rc;
    if( (p->rc = sessionInputBuffer(&p->in, 2)) ) return p->rc;
    op = p->in.aData[p->in.iNext++];
  }

  p->op = op;
  p->bIndirect = p->in.aData[p->in.iNext++];
  if( p->op!=SQLITE_UPDATE && p->op!=SQLITE_DELETE && p->op!=SQLITE_INSERT ){
    return (p->rc = SQLITE_CORRUPT_BKPT);
  }

  if( paRec ){ 
    int nVal;                     /* Number of values to buffer */
    if( p->bPatchset==0 && op==SQLITE_UPDATE ){
      nVal = p->nCol * 2;
    }else if( p->bPatchset && op==SQLITE_DELETE ){
      nVal = 0;
      for(i=0; i<p->nCol; i++) if( p->abPK[i] ) nVal++;
    }else{
      nVal = p->nCol;
    }
    p->rc = sessionChangesetBufferRecord(&p->in, nVal, pnRec);
    if( p->rc!=SQLITE_OK ) return p->rc;
    *paRec = &p->in.aData[p->in.iNext];
    p->in.iNext += *pnRec;
  }else{

    /* If this is an UPDATE or DELETE, read the old.* record. */
    if( p->op!=SQLITE_INSERT && (p->bPatchset==0 || p->op==SQLITE_DELETE) ){
      u8 *abPK = p->bPatchset ? p->abPK : 0;
      p->rc = sessionReadRecord(&p->in, p->nCol, abPK, p->apValue);
      if( p->rc!=SQLITE_OK ) return p->rc;
    }

    /* If this is an INSERT or UPDATE, read the new.* record. */
    if( p->op!=SQLITE_DELETE ){
      p->rc = sessionReadRecord(&p->in, p->nCol, 0, &p->apValue[p->nCol]);
      if( p->rc!=SQLITE_OK ) return p->rc;
    }

    if( p->bPatchset && p->op==SQLITE_UPDATE ){
      /* If this is an UPDATE that is part of a patchset, then all PK and
      ** modified fields are present in the new.* record. The old.* record
      ** is currently completely empty. This block shifts the PK fields from
      ** new.* to old.*, to accommodate the code that reads these arrays.  */
      int i;
      for(i=0; i<p->nCol; i++){
        assert( p->apValue[i]==0 );
        assert( p->abPK[i]==0 || p->apValue[i+p->nCol] );
        if( p->abPK[i] ){
          p->apValue[i] = p->apValue[i+p->nCol];
          p->apValue[i+p->nCol] = 0;
        }
      }
    }
  }

  return SQLITE_ROW;
}

/*
** Advance an iterator created by sqlite3changeset_start() to the next
** change in the changeset. This function may return SQLITE_ROW, SQLITE_DONE
** or SQLITE_CORRUPT.
**
** This function may not be called on iterators passed to a conflict handler
** callback by changeset_apply().
*/
SQLITE_API int sqlite3changeset_next(sqlite3_changeset_iter *p){
  return sessionChangesetNext(p, 0, 0);
}

/*
** The following function extracts information on the current change
** from a changeset iterator. It may only be called after changeset_next()
** has returned SQLITE_ROW.
*/
SQLITE_API int sqlite3changeset_op(
  sqlite3_changeset_iter *pIter,  /* Iterator handle */
  const char **pzTab,             /* OUT: Pointer to table name */
  int *pnCol,                     /* OUT: Number of columns in table */
  int *pOp,                       /* OUT: SQLITE_INSERT, DELETE or UPDATE */
  int *pbIndirect                 /* OUT: True if change is indirect */
){
  *pOp = pIter->op;
  *pnCol = pIter->nCol;
  *pzTab = pIter->zTab;
  if( pbIndirect ) *pbIndirect = pIter->bIndirect;
  return SQLITE_OK;
}

/*
** Return information regarding the PRIMARY KEY and number of columns in
** the database table affected by the change that pIter currently points
** to. This function may only be called after changeset_next() returns
** SQLITE_ROW.
*/
SQLITE_API int sqlite3changeset_pk(
  sqlite3_changeset_iter *pIter,  /* Iterator object */
  unsigned char **pabPK,          /* OUT: Array of boolean - true for PK cols */
  int *pnCol                      /* OUT: Number of entries in output array */
){
  *pabPK = pIter->abPK;
  if( pnCol ) *pnCol = pIter->nCol;
  return SQLITE_OK;
}

/*
** This function may only be called while the iterator is pointing to an
** SQLITE_UPDATE or SQLITE_DELETE change (see sqlite3changeset_op()).
** Otherwise, SQLITE_MISUSE is returned.
**
** It sets *ppValue to point to an sqlite3_value structure containing the
** iVal'th value in the old.* record. Or, if that particular value is not
** included in the record (because the change is an UPDATE and the field
** was not modified and is not a PK column), set *ppValue to NULL.
**
** If value iVal is out-of-range, SQLITE_RANGE is returned and *ppValue is
** not modified. Otherwise, SQLITE_OK.
*/
SQLITE_API int sqlite3changeset_old(
  sqlite3_changeset_iter *pIter,  /* Changeset iterator */
  int iVal,                       /* Index of old.* value to retrieve */
  sqlite3_value **ppValue         /* OUT: Old value (or NULL pointer) */
){
  if( pIter->op!=SQLITE_UPDATE && pIter->op!=SQLITE_DELETE ){
    return SQLITE_MISUSE;
  }
  if( iVal<0 || iVal>=pIter->nCol ){
    return SQLITE_RANGE;
  }
  *ppValue = pIter->apValue[iVal];
  return SQLITE_OK;
}

/*
** This function may only be called while the iterator is pointing to an
** SQLITE_UPDATE or SQLITE_INSERT change (see sqlite3changeset_op()).
** Otherwise, SQLITE_MISUSE is returned.
**
** It sets *ppValue to point to an sqlite3_value structure containing the
** iVal'th value in the new.* record. Or, if that particular value is not
** included in the record (because the change is an UPDATE and the field
** was not modified), set *ppValue to NULL.
**
** If value iVal is out-of-range, SQLITE_RANGE is returned and *ppValue is
** not modified. Otherwise, SQLITE_OK.
*/
SQLITE_API int sqlite3changeset_new(
  sqlite3_changeset_iter *pIter,  /* Changeset iterator */
  int iVal,                       /* Index of new.* value to retrieve */
  sqlite3_value **ppValue         /* OUT: New value (or NULL pointer) */
){
  if( pIter->op!=SQLITE_UPDATE && pIter->op!=SQLITE_INSERT ){
    return SQLITE_MISUSE;
  }
  if( iVal<0 || iVal>=pIter->nCol ){
    return SQLITE_RANGE;
  }
  *ppValue = pIter->apValue[pIter->nCol+iVal];
  return SQLITE_OK;
}

/*
** The following two macros are used internally. They are similar to the
** sqlite3changeset_new() and sqlite3changeset_old() functions, except that
** they omit all error checking and return a pointer to the requested value.
*/
#define sessionChangesetNew(pIter, iVal) (pIter)->apValue[(pIter)->nCol+(iVal)]
#define sessionChangesetOld(pIter, iVal) (pIter)->apValue[(iVal)]

/*
** This function may only be called with a changeset iterator that has been
** passed to an SQLITE_CHANGESET_DATA or SQLITE_CHANGESET_CONFLICT 
** conflict-handler function. Otherwise, SQLITE_MISUSE is returned.
**
** If successful, *ppValue is set to point to an sqlite3_value structure
** containing the iVal'th value of the conflicting record.
**
** If value iVal is out-of-range or some other error occurs, an SQLite error
** code is returned. Otherwise, SQLITE_OK.
*/
SQLITE_API int sqlite3changeset_conflict(
  sqlite3_changeset_iter *pIter,  /* Changeset iterator */
  int iVal,                       /* Index of conflict record value to fetch */
  sqlite3_value **ppValue         /* OUT: Value from conflicting row */
){
  if( !pIter->pConflict ){
    return SQLITE_MISUSE;
  }
  if( iVal<0 || iVal>=sqlite3_column_count(pIter->pConflict) ){
    return SQLITE_RANGE;
  }
  *ppValue = sqlite3_column_value(pIter->pConflict, iVal);
  return SQLITE_OK;
}

/*
** This function may only be called with an iterator passed to an
** SQLITE_CHANGESET_FOREIGN_KEY conflict handler callback. In this case
** it sets the output variable to the total number of known foreign key
** violations in the destination database and returns SQLITE_OK.
**
** In all other cases this function returns SQLITE_MISUSE.
*/
SQLITE_API int sqlite3changeset_fk_conflicts(
  sqlite3_changeset_iter *pIter,  /* Changeset iterator */
  int *pnOut                      /* OUT: Number of FK violations */
){
  if( pIter->pConflict || pIter->apValue ){
    return SQLITE_MISUSE;
  }
  *pnOut = pIter->nCol;
  return SQLITE_OK;
}


/*
** Finalize an iterator allocated with sqlite3changeset_start().
**
** This function may not be called on iterators passed to a conflict handler
** callback by changeset_apply().
*/
SQLITE_API int sqlite3changeset_finalize(sqlite3_changeset_iter *p){
  int rc = SQLITE_OK;
  if( p ){
    int i;                        /* Used to iterate through p->apValue[] */
    rc = p->rc;
    if( p->apValue ){
      for(i=0; i<p->nCol*2; i++) sqlite3ValueFree(p->apValue[i]);
    }
    sqlite3_free(p->tblhdr.aBuf);
    sqlite3_free(p->in.buf.aBuf);
    sqlite3_free(p);
  }
  return rc;
}

static int sessionChangesetInvert(
  SessionInput *pInput,           /* Input changeset */
  int (*xOutput)(void *pOut, const void *pData, int nData),
  void *pOut,
  int *pnInverted,                /* OUT: Number of bytes in output changeset */
  void **ppInverted               /* OUT: Inverse of pChangeset */
){
  int rc = SQLITE_OK;             /* Return value */
  SessionBuffer sOut;             /* Output buffer */
  int nCol = 0;                   /* Number of cols in current table */
  u8 *abPK = 0;                   /* PK array for current table */
  sqlite3_value **apVal = 0;      /* Space for values for UPDATE inversion */
  SessionBuffer sPK = {0, 0, 0};  /* PK array for current table */

  /* Initialize the output buffer */
  memset(&sOut, 0, sizeof(SessionBuffer));

  /* Zero the output variables in case an error occurs. */
  if( ppInverted ){
    *ppInverted = 0;
    *pnInverted = 0;
  }

  while( 1 ){
    u8 eType;

    /* Test for EOF. */
    if( (rc = sessionInputBuffer(pInput, 2)) ) goto finished_invert;
    if( pInput->iNext>=pInput->nData ) break;
    eType = pInput->aData[pInput->iNext];

    switch( eType ){
      case 'T': {
        /* A 'table' record consists of:
        **
        **   * A constant 'T' character,
        **   * Number of columns in said table (a varint),
        **   * An array of nCol bytes (sPK),
        **   * A nul-terminated table name.
        */
        int nByte;
        int nVar;
        pInput->iNext++;
        if( (rc = sessionChangesetBufferTblhdr(pInput, &nByte)) ){
          goto finished_invert;
        }
        nVar = sessionVarintGet(&pInput->aData[pInput->iNext], &nCol);
        sPK.nBuf = 0;
        sessionAppendBlob(&sPK, &pInput->aData[pInput->iNext+nVar], nCol, &rc);
        sessionAppendByte(&sOut, eType, &rc);
        sessionAppendBlob(&sOut, &pInput->aData[pInput->iNext], nByte, &rc);
        if( rc ) goto finished_invert;

        pInput->iNext += nByte;
        sqlite3_free(apVal);
        apVal = 0;
        abPK = sPK.aBuf;
        break;
      }

      case SQLITE_INSERT:
      case SQLITE_DELETE: {
        int nByte;
        int bIndirect = pInput->aData[pInput->iNext+1];
        int eType2 = (eType==SQLITE_DELETE ? SQLITE_INSERT : SQLITE_DELETE);
        pInput->iNext += 2;
        assert( rc==SQLITE_OK );
        rc = sessionChangesetBufferRecord(pInput, nCol, &nByte);
        sessionAppendByte(&sOut, eType2, &rc);
        sessionAppendByte(&sOut, bIndirect, &rc);
        sessionAppendBlob(&sOut, &pInput->aData[pInput->iNext], nByte, &rc);
        pInput->iNext += nByte;
        if( rc ) goto finished_invert;
        break;
      }

      case SQLITE_UPDATE: {
        int iCol;

        if( 0==apVal ){
          apVal = (sqlite3_value **)sqlite3_malloc(sizeof(apVal[0])*nCol*2);
          if( 0==apVal ){
            rc = SQLITE_NOMEM;
            goto finished_invert;
          }
          memset(apVal, 0, sizeof(apVal[0])*nCol*2);
        }

        /* Write the header for the new UPDATE change. Same as the original. */
        sessionAppendByte(&sOut, eType, &rc);
        sessionAppendByte(&sOut, pInput->aData[pInput->iNext+1], &rc);

        /* Read the old.* and new.* records for the update change. */
        pInput->iNext += 2;
        rc = sessionReadRecord(pInput, nCol, 0, &apVal[0]);
        if( rc==SQLITE_OK ){
          rc = sessionReadRecord(pInput, nCol, 0, &apVal[nCol]);
        }

        /* Write the new old.* record. Consists of the PK columns from the
        ** original old.* record, and the other values from the original
        ** new.* record. */
        for(iCol=0; iCol<nCol; iCol++){
          sqlite3_value *pVal = apVal[iCol + (abPK[iCol] ? 0 : nCol)];
          sessionAppendValue(&sOut, pVal, &rc);
        }

        /* Write the new new.* record. Consists of a copy of all values
        ** from the original old.* record, except for the PK columns, which
        ** are set to "undefined". */
        for(iCol=0; iCol<nCol; iCol++){
          sqlite3_value *pVal = (abPK[iCol] ? 0 : apVal[iCol]);
          sessionAppendValue(&sOut, pVal, &rc);
        }

        for(iCol=0; iCol<nCol*2; iCol++){
          sqlite3ValueFree(apVal[iCol]);
        }
        memset(apVal, 0, sizeof(apVal[0])*nCol*2);
        if( rc!=SQLITE_OK ){
          goto finished_invert;
        }

        break;
      }

      default:
        rc = SQLITE_CORRUPT_BKPT;
        goto finished_invert;
    }

    assert( rc==SQLITE_OK );
    if( xOutput && sOut.nBuf>=SESSIONS_STRM_CHUNK_SIZE ){
      rc = xOutput(pOut, sOut.aBuf, sOut.nBuf);
      sOut.nBuf = 0;
      if( rc!=SQLITE_OK ) goto finished_invert;
    }
  }

  assert( rc==SQLITE_OK );
  if( pnInverted ){
    *pnInverted = sOut.nBuf;
    *ppInverted = sOut.aBuf;
    sOut.aBuf = 0;
  }else if( sOut.nBuf>0 ){
    rc = xOutput(pOut, sOut.aBuf, sOut.nBuf);
  }

 finished_invert:
  sqlite3_free(sOut.aBuf);
  sqlite3_free(apVal);
  sqlite3_free(sPK.aBuf);
  return rc;
}


/*
** Invert a changeset object.
*/
SQLITE_API int sqlite3changeset_invert(
  int nChangeset,                 /* Number of bytes in input */
  const void *pChangeset,         /* Input changeset */
  int *pnInverted,                /* OUT: Number of bytes in output changeset */
  void **ppInverted               /* OUT: Inverse of pChangeset */
){
  SessionInput sInput;

  /* Set up the input stream */
  memset(&sInput, 0, sizeof(SessionInput));
  sInput.nData = nChangeset;
  sInput.aData = (u8*)pChangeset;

  return sessionChangesetInvert(&sInput, 0, 0, pnInverted, ppInverted);
}

/*
** Streaming version of sqlite3changeset_invert().
*/
SQLITE_API int sqlite3changeset_invert_strm(
  int (*xInput)(void *pIn, void *pData, int *pnData),
  void *pIn,
  int (*xOutput)(void *pOut, const void *pData, int nData),
  void *pOut
){
  SessionInput sInput;
  int rc;

  /* Set up the input stream */
  memset(&sInput, 0, sizeof(SessionInput));
  sInput.xInput = xInput;
  sInput.pIn = pIn;

  rc = sessionChangesetInvert(&sInput, xOutput, pOut, 0, 0);
  sqlite3_free(sInput.buf.aBuf);
  return rc;
}

typedef struct SessionApplyCtx SessionApplyCtx;
struct SessionApplyCtx {
  sqlite3 *db;
  sqlite3_stmt *pDelete;          /* DELETE statement */
  sqlite3_stmt *pUpdate;          /* UPDATE statement */
  sqlite3_stmt *pInsert;          /* INSERT statement */
  sqlite3_stmt *pSelect;          /* SELECT statement */
  int nCol;                       /* Size of azCol[] and abPK[] arrays */
  const char **azCol;             /* Array of column names */
  u8 *abPK;                       /* Boolean array - true if column is in PK */
};

/*
** Formulate a statement to DELETE a row from database db. Assuming a table
** structure like this:
**
**     CREATE TABLE x(a, b, c, d, PRIMARY KEY(a, c));
**
** The DELETE statement looks like this:
**
**     DELETE FROM x WHERE a = :1 AND c = :3 AND (:5 OR b IS :2 AND d IS :4)
**
** Variable :5 (nCol+1) is a boolean. It should be set to 0 if we require
** matching b and d values, or 1 otherwise. The second case comes up if the
** conflict handler is invoked with NOTFOUND and returns CHANGESET_REPLACE.
**
** If successful, SQLITE_OK is returned and SessionApplyCtx.pDelete is left
** pointing to the prepared version of the SQL statement.
*/
static int sessionDeleteRow(
  sqlite3 *db,                    /* Database handle */
  const char *zTab,               /* Table name */
  SessionApplyCtx *p              /* Session changeset-apply context */
){
  int i;
  const char *zSep = "";
  int rc = SQLITE_OK;
  SessionBuffer buf = {0, 0, 0};
  int nPk = 0;

  sessionAppendStr(&buf, "DELETE FROM ", &rc);
  sessionAppendIdent(&buf, zTab, &rc);
  sessionAppendStr(&buf, " WHERE ", &rc);

  for(i=0; i<p->nCol; i++){
    if( p->abPK[i] ){
      nPk++;
      sessionAppendStr(&buf, zSep, &rc);
      sessionAppendIdent(&buf, p->azCol[i], &rc);
      sessionAppendStr(&buf, " = ?", &rc);
      sessionAppendInteger(&buf, i+1, &rc);
      zSep = " AND ";
    }
  }

  if( nPk<p->nCol ){
    sessionAppendStr(&buf, " AND (?", &rc);
    sessionAppendInteger(&buf, p->nCol+1, &rc);
    sessionAppendStr(&buf, " OR ", &rc);

    zSep = "";
    for(i=0; i<p->nCol; i++){
      if( !p->abPK[i] ){
        sessionAppendStr(&buf, zSep, &rc);
        sessionAppendIdent(&buf, p->azCol[i], &rc);
        sessionAppendStr(&buf, " IS ?", &rc);
        sessionAppendInteger(&buf, i+1, &rc);
        zSep = "AND ";
      }
    }
    sessionAppendStr(&buf, ")", &rc);
  }

  if( rc==SQLITE_OK ){
    rc = sqlite3_prepare_v2(db, (char *)buf.aBuf, buf.nBuf, &p->pDelete, 0);
  }
  sqlite3_free(buf.aBuf);

  return rc;
}

/*
** Formulate and prepare a statement to UPDATE a row from database db. 
** Assuming a table structure like this:
**
**     CREATE TABLE x(a, b, c, d, PRIMARY KEY(a, c));
**
** The UPDATE statement looks like this:
**
**     UPDATE x SET
**     a = CASE WHEN ?2  THEN ?3  ELSE a END,
**     b = CASE WHEN ?5  THEN ?6  ELSE b END,
**     c = CASE WHEN ?8  THEN ?9  ELSE c END,
**     d = CASE WHEN ?11 THEN ?12 ELSE d END
**     WHERE a = ?1 AND c = ?7 AND (?13 OR 
**       (?5==0 OR b IS ?4) AND (?11==0 OR d IS ?10) AND
**     )
**
** For each column in the table, there are three variables to bind:
**
**     ?(i*3+1)    The old.* value of the column, if any.
**     ?(i*3+2)    A boolean flag indicating that the value is being modified.
**     ?(i*3+3)    The new.* value of the column, if any.
**
** Also, a boolean flag that, if set to true, causes the statement to update
** a row even if the non-PK values do not match. This is required if the
** conflict-handler is invoked with CHANGESET_DATA and returns
** CHANGESET_REPLACE. This is variable "?(nCol*3+1)".
**
** If successful, SQLITE_OK is returned and SessionApplyCtx.pUpdate is left
** pointing to the prepared version of the SQL statement.
*/
static int sessionUpdateRow(
  sqlite3 *db,                    /* Database handle */
  const char *zTab,               /* Table name */
  SessionApplyCtx *p              /* Session changeset-apply context */
){
  int rc = SQLITE_OK;
  int i;
  const char *zSep = "";
  SessionBuffer buf = {0, 0, 0};

  /* Append "UPDATE tbl SET " */
  sessionAppendStr(&buf, "UPDATE ", &rc);
  sessionAppendIdent(&buf, zTab, &rc);
  sessionAppendStr(&buf, " SET ", &rc);

  /* Append the assignments */
  for(i=0; i<p->nCol; i++){
    sessionAppendStr(&buf, zSep, &rc);
    sessionAppendIdent(&buf, p->azCol[i], &rc);
    sessionAppendStr(&buf, " = CASE WHEN ?", &rc);
    sessionAppendInteger(&buf, i*3+2, &rc);
    sessionAppendStr(&buf, " THEN ?", &rc);
    sessionAppendInteger(&buf, i*3+3, &rc);
    sessionAppendStr(&buf, " ELSE ", &rc);
    sessionAppendIdent(&buf, p->azCol[i], &rc);
    sessionAppendStr(&buf, " END", &rc);
    zSep = ", ";
  }

  /* Append the PK part of the WHERE clause */
  sessionAppendStr(&buf, " WHERE ", &rc);
  for(i=0; i<p->nCol; i++){
    if( p->abPK[i] ){
      sessionAppendIdent(&buf, p->azCol[i], &rc);
      sessionAppendStr(&buf, " = ?", &rc);
      sessionAppendInteger(&buf, i*3+1, &rc);
      sessionAppendStr(&buf, " AND ", &rc);
    }
  }

  /* Append the non-PK part of the WHERE clause */
  sessionAppendStr(&buf, " (?", &rc);
  sessionAppendInteger(&buf, p->nCol*3+1, &rc);
  sessionAppendStr(&buf, " OR 1", &rc);
  for(i=0; i<p->nCol; i++){
    if( !p->abPK[i] ){
      sessionAppendStr(&buf, " AND (?", &rc);
      sessionAppendInteger(&buf, i*3+2, &rc);
      sessionAppendStr(&buf, "=0 OR ", &rc);
      sessionAppendIdent(&buf, p->azCol[i], &rc);
      sessionAppendStr(&buf, " IS ?", &rc);
      sessionAppendInteger(&buf, i*3+1, &rc);
      sessionAppendStr(&buf, ")", &rc);
    }
  }
  sessionAppendStr(&buf, ")", &rc);

  if( rc==SQLITE_OK ){
    rc = sqlite3_prepare_v2(db, (char *)buf.aBuf, buf.nBuf, &p->pUpdate, 0);
  }
  sqlite3_free(buf.aBuf);

  return rc;
}

/*
** Formulate and prepare an SQL statement to query table zTab by primary
** key. Assuming the following table structure:
**
**     CREATE TABLE x(a, b, c, d, PRIMARY KEY(a, c));
**
** The SELECT statement looks like this:
**
**     SELECT * FROM x WHERE a = ?1 AND c = ?3
**
** If successful, SQLITE_OK is returned and SessionApplyCtx.pSelect is left
** pointing to the prepared version of the SQL statement.
*/
static int sessionSelectRow(
  sqlite3 *db,                    /* Database handle */
  const char *zTab,               /* Table name */
  SessionApplyCtx *p              /* Session changeset-apply context */
){
  return sessionSelectStmt(
      db, "main", zTab, p->nCol, p->azCol, p->abPK, &p->pSelect);
}

/*
** Formulate and prepare an INSERT statement to add a record to table zTab.
** For example:
**
**     INSERT INTO main."zTab" VALUES(?1, ?2, ?3 ...);
**
** If successful, SQLITE_OK is returned and SessionApplyCtx.pInsert is left
** pointing to the prepared version of the SQL statement.
*/
static int sessionInsertRow(
  sqlite3 *db,                    /* Database handle */
  const char *zTab,               /* Table name */
  SessionApplyCtx *p              /* Session changeset-apply context */
){
  int rc = SQLITE_OK;
  int i;
  SessionBuffer buf = {0, 0, 0};

  sessionAppendStr(&buf, "INSERT INTO main.", &rc);
  sessionAppendIdent(&buf, zTab, &rc);
  sessionAppendStr(&buf, " VALUES(?", &rc);
  for(i=1; i<p->nCol; i++){
    sessionAppendStr(&buf, ", ?", &rc);
  }
  sessionAppendStr(&buf, ")", &rc);

  if( rc==SQLITE_OK ){
    rc = sqlite3_prepare_v2(db, (char *)buf.aBuf, buf.nBuf, &p->pInsert, 0);
  }
  sqlite3_free(buf.aBuf);
  return rc;
}

/*
** A wrapper around sqlite3_bind_value() that detects an extra problem. 
** See comments in the body of this function for details.
*/
static int sessionBindValue(
  sqlite3_stmt *pStmt,            /* Statement to bind value to */
  int i,                          /* Parameter number to bind to */
  sqlite3_value *pVal             /* Value to bind */
){
  int eType = sqlite3_value_type(pVal);
  /* COVERAGE: The (pVal->z==0) branch is never true using current versions
  ** of SQLite. If a malloc fails in an sqlite3_value_xxx() function, either
  ** the (pVal->z) variable remains as it was or the type of the value is
  ** set to SQLITE_NULL.  */
  if( (eType==SQLITE_TEXT || eType==SQLITE_BLOB) && pVal->z==0 ){
    /* This condition occurs when an earlier OOM in a call to
    ** sqlite3_value_text() or sqlite3_value_blob() (perhaps from within
    ** a conflict-handler) has zeroed the pVal->z pointer. Return NOMEM. */
    return SQLITE_NOMEM;
  }
  return sqlite3_bind_value(pStmt, i, pVal);
}

/*
** Iterator pIter must point to an SQLITE_INSERT entry. This function 
** transfers new.* values from the current iterator entry to statement
** pStmt. The table being inserted into has nCol columns.
**
** New.* value $i 0 from the iterator is bound to variable ($i+1) of 
** statement pStmt. If parameter abPK is NULL, all values from 0 to (nCol-1)
** are transfered to the statement. Otherwise, if abPK is not NULL, it points
** to an array nCol elements in size. In this case only those values for 
** which abPK[$i] is true are read from the iterator and bound to the 
** statement.
**
** An SQLite error code is returned if an error occurs. Otherwise, SQLITE_OK.
*/
static int sessionBindRow(
  sqlite3_changeset_iter *pIter,  /* Iterator to read values from */
  int(*xValue)(sqlite3_changeset_iter *, int, sqlite3_value **),
  int nCol,                       /* Number of columns */
  u8 *abPK,                       /* If not NULL, bind only if true */
  sqlite3_stmt *pStmt             /* Bind values to this statement */
){
  int i;
  int rc = SQLITE_OK;

  /* Neither sqlite3changeset_old or sqlite3changeset_new can fail if the
  ** argument iterator points to a suitable entry. Make sure that xValue 
  ** is one of these to guarantee that it is safe to ignore the return 
  ** in the code below. */
  assert( xValue==sqlite3changeset_old || xValue==sqlite3changeset_new );

  for(i=0; rc==SQLITE_OK && i<nCol; i++){
    if( !abPK || abPK[i] ){
      sqlite3_value *pVal;
      (void)xValue(pIter, i, &pVal);
      rc = sessionBindValue(pStmt, i+1, pVal);
    }
  }
  return rc;
}

/*
** SQL statement pSelect is as generated by the sessionSelectRow() function.
** This function binds the primary key values from the change that changeset
** iterator pIter points to to the SELECT and attempts to seek to the table
** entry. If a row is found, the SELECT statement left pointing at the row 
** and SQLITE_ROW is returned. Otherwise, if no row is found and no error
** has occured, the statement is reset and SQLITE_OK is returned. If an
** error occurs, the statement is reset and an SQLite error code is returned.
**
** If this function returns SQLITE_ROW, the caller must eventually reset() 
** statement pSelect. If any other value is returned, the statement does
** not require a reset().
**
** If the iterator currently points to an INSERT record, bind values from the
** new.* record to the SELECT statement. Or, if it points to a DELETE or
** UPDATE, bind values from the old.* record. 
*/
static int sessionSeekToRow(
  sqlite3 *db,                    /* Database handle */
  sqlite3_changeset_iter *pIter,  /* Changeset iterator */
  u8 *abPK,                       /* Primary key flags array */
  sqlite3_stmt *pSelect           /* SELECT statement from sessionSelectRow() */
){
  int rc;                         /* Return code */
  int nCol;                       /* Number of columns in table */
  int op;                         /* Changset operation (SQLITE_UPDATE etc.) */
  const char *zDummy;             /* Unused */

  sqlite3changeset_op(pIter, &zDummy, &nCol, &op, 0);
  rc = sessionBindRow(pIter, 
      op==SQLITE_INSERT ? sqlite3changeset_new : sqlite3changeset_old,
      nCol, abPK, pSelect
  );

  if( rc==SQLITE_OK ){
    rc = sqlite3_step(pSelect);
    if( rc!=SQLITE_ROW ) rc = sqlite3_reset(pSelect);
  }

  return rc;
}

/*
** Invoke the conflict handler for the change that the changeset iterator
** currently points to.
**
** Argument eType must be either CHANGESET_DATA or CHANGESET_CONFLICT.
** If argument pbReplace is NULL, then the type of conflict handler invoked
** depends solely on eType, as follows:
**
**    eType value                 Value passed to xConflict
**    -------------------------------------------------
**    CHANGESET_DATA              CHANGESET_NOTFOUND
**    CHANGESET_CONFLICT          CHANGESET_CONSTRAINT
**
** Or, if pbReplace is not NULL, then an attempt is made to find an existing
** record with the same primary key as the record about to be deleted, updated
** or inserted. If such a record can be found, it is available to the conflict
** handler as the "conflicting" record. In this case the type of conflict
** handler invoked is as follows:
**
**    eType value         PK Record found?   Value passed to xConflict
**    ----------------------------------------------------------------
**    CHANGESET_DATA      Yes                CHANGESET_DATA
**    CHANGESET_DATA      No                 CHANGESET_NOTFOUND
**    CHANGESET_CONFLICT  Yes                CHANGESET_CONFLICT
**    CHANGESET_CONFLICT  No                 CHANGESET_CONSTRAINT
**
** If pbReplace is not NULL, and a record with a matching PK is found, and
** the conflict handler function returns SQLITE_CHANGESET_REPLACE, *pbReplace
** is set to non-zero before returning SQLITE_OK.
**
** If the conflict handler returns SQLITE_CHANGESET_ABORT, SQLITE_ABORT is
** returned. Or, if the conflict handler returns an invalid value, 
** SQLITE_MISUSE. If the conflict handler returns SQLITE_CHANGESET_OMIT,
** this function returns SQLITE_OK.
*/
static int sessionConflictHandler(
  int eType,                      /* Either CHANGESET_DATA or CONFLICT */
  SessionApplyCtx *p,             /* changeset_apply() context */
  sqlite3_changeset_iter *pIter,  /* Changeset iterator */
  int(*xConflict)(void *, int, sqlite3_changeset_iter*),
  void *pCtx,                     /* First argument for conflict handler */
  int *pbReplace                  /* OUT: Set to true if PK row is found */
){
  int res;                        /* Value returned by conflict handler */
  int rc;
  int nCol;
  int op;
  const char *zDummy;

  sqlite3changeset_op(pIter, &zDummy, &nCol, &op, 0);

  assert( eType==SQLITE_CHANGESET_CONFLICT || eType==SQLITE_CHANGESET_DATA );
  assert( SQLITE_CHANGESET_CONFLICT+1==SQLITE_CHANGESET_CONSTRAINT );
  assert( SQLITE_CHANGESET_DATA+1==SQLITE_CHANGESET_NOTFOUND );

  /* Bind the new.* PRIMARY KEY values to the SELECT statement. */
  if( pbReplace ){
    rc = sessionSeekToRow(p->db, pIter, p->abPK, p->pSelect);
  }else{
    rc = SQLITE_OK;
  }

  if( rc==SQLITE_ROW ){
    /* There exists another row with the new.* primary key. */
    pIter->pConflict = p->pSelect;
    res = xConflict(pCtx, eType, pIter);
    pIter->pConflict = 0;
    rc = sqlite3_reset(p->pSelect);
  }else if( rc==SQLITE_OK ){
    /* No other row with the new.* primary key. */
    res = xConflict(pCtx, eType+1, pIter);
    if( res==SQLITE_CHANGESET_REPLACE ) rc = SQLITE_MISUSE;
  }

  if( rc==SQLITE_OK ){
    switch( res ){
      case SQLITE_CHANGESET_REPLACE:
        assert( pbReplace );
        *pbReplace = 1;
        break;

      case SQLITE_CHANGESET_OMIT:
        break;

      case SQLITE_CHANGESET_ABORT:
        rc = SQLITE_ABORT;
        break;

      default:
        rc = SQLITE_MISUSE;
        break;
    }
  }

  return rc;
}

/*
** Attempt to apply the change that the iterator passed as the first argument
** currently points to to the database. If a conflict is encountered, invoke
** the conflict handler callback.
**
** If argument pbRetry is NULL, then ignore any CHANGESET_DATA conflict. If
** one is encountered, update or delete the row with the matching primary key
** instead. Or, if pbRetry is not NULL and a CHANGESET_DATA conflict occurs,
** invoke the conflict handler. If it returns CHANGESET_REPLACE, set *pbRetry
** to true before returning. In this case the caller will invoke this function
** again, this time with pbRetry set to NULL.
**
** If argument pbReplace is NULL and a CHANGESET_CONFLICT conflict is 
** encountered invoke the conflict handler with CHANGESET_CONSTRAINT instead.
** Or, if pbReplace is not NULL, invoke it with CHANGESET_CONFLICT. If such
** an invocation returns SQLITE_CHANGESET_REPLACE, set *pbReplace to true
** before retrying. In this case the caller attempts to remove the conflicting
** row before invoking this function again, this time with pbReplace set 
** to NULL.
**
** If any conflict handler returns SQLITE_CHANGESET_ABORT, this function
** returns SQLITE_ABORT. Otherwise, if no error occurs, SQLITE_OK is 
** returned.
*/
static int sessionApplyOneOp(
  sqlite3_changeset_iter *pIter,  /* Changeset iterator */
  SessionApplyCtx *p,             /* changeset_apply() context */
  int(*xConflict)(void *, int, sqlite3_changeset_iter *),
  void *pCtx,                     /* First argument for the conflict handler */
  int *pbReplace,                 /* OUT: True to remove PK row and retry */
  int *pbRetry                    /* OUT: True to retry. */
){
  const char *zDummy;
  int op;
  int nCol;
  int rc = SQLITE_OK;

  assert( p->pDelete && p->pUpdate && p->pInsert && p->pSelect );
  assert( p->azCol && p->abPK );
  assert( !pbReplace || *pbReplace==0 );

  sqlite3changeset_op(pIter, &zDummy, &nCol, &op, 0);

  if( op==SQLITE_DELETE ){

    /* Bind values to the DELETE statement. If conflict handling is required,
    ** bind values for all columns and set bound variable (nCol+1) to true.
    ** Or, if conflict handling is not required, bind just the PK column
    ** values and, if it exists, set (nCol+1) to false. Conflict handling
    ** is not required if:
    **
    **   * this is a patchset, or
    **   * (pbRetry==0), or
    **   * all columns of the table are PK columns (in this case there is
    **     no (nCol+1) variable to bind to).
    */
    u8 *abPK = (pIter->bPatchset ? p->abPK : 0);
    rc = sessionBindRow(pIter, sqlite3changeset_old, nCol, abPK, p->pDelete);
    if( rc==SQLITE_OK && sqlite3_bind_parameter_count(p->pDelete)>nCol ){
      rc = sqlite3_bind_int(p->pDelete, nCol+1, (pbRetry==0 || abPK));
    }
    if( rc!=SQLITE_OK ) return rc;

    sqlite3_step(p->pDelete);
    rc = sqlite3_reset(p->pDelete);
    if( rc==SQLITE_OK && sqlite3_changes(p->db)==0 ){
      rc = sessionConflictHandler(
          SQLITE_CHANGESET_DATA, p, pIter, xConflict, pCtx, pbRetry
      );
    }else if( (rc&0xff)==SQLITE_CONSTRAINT ){
      rc = sessionConflictHandler(
          SQLITE_CHANGESET_CONFLICT, p, pIter, xConflict, pCtx, 0
      );
    }

  }else if( op==SQLITE_UPDATE ){
    int i;

    /* Bind values to the UPDATE statement. */
    for(i=0; rc==SQLITE_OK && i<nCol; i++){
      sqlite3_value *pOld = sessionChangesetOld(pIter, i);
      sqlite3_value *pNew = sessionChangesetNew(pIter, i);

      sqlite3_bind_int(p->pUpdate, i*3+2, !!pNew);
      if( pOld ){
        rc = sessionBindValue(p->pUpdate, i*3+1, pOld);
      }
      if( rc==SQLITE_OK && pNew ){
        rc = sessionBindValue(p->pUpdate, i*3+3, pNew);
      }
    }
    if( rc==SQLITE_OK ){
      sqlite3_bind_int(p->pUpdate, nCol*3+1, pbRetry==0 || pIter->bPatchset);
    }
    if( rc!=SQLITE_OK ) return rc;

    /* Attempt the UPDATE. In the case of a NOTFOUND or DATA conflict,
    ** the result will be SQLITE_OK with 0 rows modified. */
    sqlite3_step(p->pUpdate);
    rc = sqlite3_reset(p->pUpdate);

    if( rc==SQLITE_OK && sqlite3_changes(p->db)==0 ){
      /* A NOTFOUND or DATA error. Search the table to see if it contains
      ** a row with a matching primary key. If so, this is a DATA conflict.
      ** Otherwise, if there is no primary key match, it is a NOTFOUND. */

      rc = sessionConflictHandler(
          SQLITE_CHANGESET_DATA, p, pIter, xConflict, pCtx, pbRetry
      );

    }else if( (rc&0xff)==SQLITE_CONSTRAINT ){
      /* This is always a CONSTRAINT conflict. */
      rc = sessionConflictHandler(
          SQLITE_CHANGESET_CONFLICT, p, pIter, xConflict, pCtx, 0
      );
    }

  }else{
    assert( op==SQLITE_INSERT );
    rc = sessionBindRow(pIter, sqlite3changeset_new, nCol, 0, p->pInsert);
    if( rc!=SQLITE_OK ) return rc;

    sqlite3_step(p->pInsert);
    rc = sqlite3_reset(p->pInsert);
    if( (rc&0xff)==SQLITE_CONSTRAINT ){
      rc = sessionConflictHandler(
          SQLITE_CHANGESET_CONFLICT, p, pIter, xConflict, pCtx, pbReplace
      );
    }
  }

  return rc;
}

/*
** Argument pIter is a changeset iterator that has been initialized, but
** not yet passed to sqlite3changeset_next(). This function applies the 
** changeset to the main database attached to handle "db". The supplied
** conflict handler callback is invoked to resolve any conflicts encountered
** while applying the change.
*/
static int sessionChangesetApply(
  sqlite3 *db,                    /* Apply change to "main" db of this handle */
  sqlite3_changeset_iter *pIter,  /* Changeset to apply */
  int(*xFilter)(
    void *pCtx,                   /* Copy of sixth arg to _apply() */
    const char *zTab              /* Table name */
  ),
  int(*xConflict)(
    void *pCtx,                   /* Copy of fifth arg to _apply() */
    int eConflict,                /* DATA, MISSING, CONFLICT, CONSTRAINT */
    sqlite3_changeset_iter *p     /* Handle describing change and conflict */
  ),
  void *pCtx                      /* First argument passed to xConflict */
){
  int schemaMismatch = 0;
  int rc;                         /* Return code */
  const char *zTab = 0;           /* Name of current table */
  int nTab = 0;                   /* Result of sqlite3Strlen30(zTab) */
  SessionApplyCtx sApply;         /* changeset_apply() context object */

  assert( xConflict!=0 );

  memset(&sApply, 0, sizeof(sApply));
  sqlite3_mutex_enter(sqlite3_db_mutex(db));
  rc = sqlite3_exec(db, "SAVEPOINT changeset_apply", 0, 0, 0);
  if( rc==SQLITE_OK ){
    rc = sqlite3_exec(db, "PRAGMA defer_foreign_keys = 1", 0, 0, 0);
  }
  while( rc==SQLITE_OK && SQLITE_ROW==sqlite3changeset_next(pIter) ){
    int nCol;
    int op;
    int bReplace = 0;
    int bRetry = 0;
    const char *zNew;
    
    sqlite3changeset_op(pIter, &zNew, &nCol, &op, 0);

    if( zTab==0 || sqlite3_strnicmp(zNew, zTab, nTab+1) ){
      u8 *abPK;

      sqlite3_free((char*)sApply.azCol);  /* cast works around VC++ bug */
      sqlite3_finalize(sApply.pDelete);
      sqlite3_finalize(sApply.pUpdate); 
      sqlite3_finalize(sApply.pInsert);
      sqlite3_finalize(sApply.pSelect);
      memset(&sApply, 0, sizeof(sApply));
      sApply.db = db;

      /* If an xFilter() callback was specified, invoke it now. If the 
      ** xFilter callback returns zero, skip this table. If it returns
      ** non-zero, proceed. */
      schemaMismatch = (xFilter && (0==xFilter(pCtx, zNew)));
      if( schemaMismatch ){
        zTab = sqlite3_mprintf("%s", zNew);
        if( zTab==0 ){
          rc = SQLITE_NOMEM;
          break;
        }
        nTab = (int)strlen(zTab);
        sApply.azCol = (const char **)zTab;
      }else{
        sqlite3changeset_pk(pIter, &abPK, 0);
        rc = sessionTableInfo(
            db, "main", zNew, &sApply.nCol, &zTab, &sApply.azCol, &sApply.abPK
        );
        if( rc!=SQLITE_OK ) break;
  
        if( sApply.nCol==0 ){
          schemaMismatch = 1;
          sqlite3_log(SQLITE_SCHEMA, 
              "sqlite3changeset_apply(): no such table: %s", zTab
          );
        }
        else if( sApply.nCol!=nCol ){
          schemaMismatch = 1;
          sqlite3_log(SQLITE_SCHEMA, 
              "sqlite3changeset_apply(): table %s has %d columns, expected %d", 
              zTab, sApply.nCol, nCol
          );
        }
        else if( memcmp(sApply.abPK, abPK, nCol)!=0 ){
          schemaMismatch = 1;
          sqlite3_log(SQLITE_SCHEMA, "sqlite3changeset_apply(): "
              "primary key mismatch for table %s", zTab
          );
        }
        else if( 
            (rc = sessionSelectRow(db, zTab, &sApply))
         || (rc = sessionUpdateRow(db, zTab, &sApply))
         || (rc = sessionDeleteRow(db, zTab, &sApply))
         || (rc = sessionInsertRow(db, zTab, &sApply))
        ){
          break;
        }
        nTab = sqlite3Strlen30(zTab);
      }
    }

    /* If there is a schema mismatch on the current table, proceed to the
    ** next change. A log message has already been issued. */
    if( schemaMismatch ) continue;

    rc = sessionApplyOneOp(pIter, &sApply, xConflict, pCtx, &bReplace, &bRetry);

    if( rc==SQLITE_OK && bRetry ){
      rc = sessionApplyOneOp(pIter, &sApply, xConflict, pCtx, &bReplace, 0);
    }

    if( bReplace ){
      assert( pIter->op==SQLITE_INSERT );
      rc = sqlite3_exec(db, "SAVEPOINT replace_op", 0, 0, 0);
      if( rc==SQLITE_OK ){
        rc = sessionBindRow(pIter, 
            sqlite3changeset_new, sApply.nCol, sApply.abPK, sApply.pDelete);
        sqlite3_bind_int(sApply.pDelete, sApply.nCol+1, 1);
      }
      if( rc==SQLITE_OK ){
        sqlite3_step(sApply.pDelete);
        rc = sqlite3_reset(sApply.pDelete);
      }
      if( rc==SQLITE_OK ){
        rc = sessionApplyOneOp(pIter, &sApply, xConflict, pCtx, 0, 0);
      }
      if( rc==SQLITE_OK ){
        rc = sqlite3_exec(db, "RELEASE replace_op", 0, 0, 0);
      }
    }
  }

  if( rc==SQLITE_OK ){
    rc = sqlite3changeset_finalize(pIter);
  }else{
    sqlite3changeset_finalize(pIter);
  }

  if( rc==SQLITE_OK ){
    int nFk, notUsed;
    sqlite3_db_status(db, SQLITE_DBSTATUS_DEFERRED_FKS, &nFk, &notUsed, 0);
    if( nFk!=0 ){
      int res = SQLITE_CHANGESET_ABORT;
      sqlite3_changeset_iter sIter;
      memset(&sIter, 0, sizeof(sIter));
      sIter.nCol = nFk;
      res = xConflict(pCtx, SQLITE_CHANGESET_FOREIGN_KEY, &sIter);
      if( res!=SQLITE_CHANGESET_OMIT ){
        rc = SQLITE_CONSTRAINT;
      }
    }
  }
  sqlite3_exec(db, "PRAGMA defer_foreign_keys = 0", 0, 0, 0);

  if( rc==SQLITE_OK ){
    rc = sqlite3_exec(db, "RELEASE changeset_apply", 0, 0, 0);
  }else{
    sqlite3_exec(db, "ROLLBACK TO changeset_apply", 0, 0, 0);
    sqlite3_exec(db, "RELEASE changeset_apply", 0, 0, 0);
  }

  sqlite3_finalize(sApply.pInsert);
  sqlite3_finalize(sApply.pDelete);
  sqlite3_finalize(sApply.pUpdate);
  sqlite3_finalize(sApply.pSelect);
  sqlite3_free((char*)sApply.azCol);  /* cast works around VC++ bug */
  sqlite3_mutex_leave(sqlite3_db_mutex(db));
  return rc;
}

/*
** Apply the changeset passed via pChangeset/nChangeset to the main database
** attached to handle "db". Invoke the supplied conflict handler callback
** to resolve any conflicts encountered while applying the change.
*/
SQLITE_API int sqlite3changeset_apply(
  sqlite3 *db,                    /* Apply change to "main" db of this handle */
  int nChangeset,                 /* Size of changeset in bytes */
  void *pChangeset,               /* Changeset blob */
  int(*xFilter)(
    void *pCtx,                   /* Copy of sixth arg to _apply() */
    const char *zTab              /* Table name */
  ),
  int(*xConflict)(
    void *pCtx,                   /* Copy of fifth arg to _apply() */
    int eConflict,                /* DATA, MISSING, CONFLICT, CONSTRAINT */
    sqlite3_changeset_iter *p     /* Handle describing change and conflict */
  ),
  void *pCtx                      /* First argument passed to xConflict */
){
  sqlite3_changeset_iter *pIter;  /* Iterator to skip through changeset */  
  int rc = sqlite3changeset_start(&pIter, nChangeset, pChangeset);
  if( rc==SQLITE_OK ){
    rc = sessionChangesetApply(db, pIter, xFilter, xConflict, pCtx);
  }
  return rc;
}

/*
** Apply the changeset passed via xInput/pIn to the main database
** attached to handle "db". Invoke the supplied conflict handler callback
** to resolve any conflicts encountered while applying the change.
*/
SQLITE_API int sqlite3changeset_apply_strm(
  sqlite3 *db,                    /* Apply change to "main" db of this handle */
  int (*xInput)(void *pIn, void *pData, int *pnData), /* Input function */
  void *pIn,                                          /* First arg for xInput */
  int(*xFilter)(
    void *pCtx,                   /* Copy of sixth arg to _apply() */
    const char *zTab              /* Table name */
  ),
  int(*xConflict)(
    void *pCtx,                   /* Copy of sixth arg to _apply() */
    int eConflict,                /* DATA, MISSING, CONFLICT, CONSTRAINT */
    sqlite3_changeset_iter *p     /* Handle describing change and conflict */
  ),
  void *pCtx                      /* First argument passed to xConflict */
){
  sqlite3_changeset_iter *pIter;  /* Iterator to skip through changeset */  
  int rc = sqlite3changeset_start_strm(&pIter, xInput, pIn);
  if( rc==SQLITE_OK ){
    rc = sessionChangesetApply(db, pIter, xFilter, xConflict, pCtx);
  }
  return rc;
}

/*
** This function is called to merge two changes to the same row together as
** part of an sqlite3changeset_concat() operation. A new change object is
** allocated and a pointer to it stored in *ppNew.
*/
static int sessionChangeMerge(
  SessionTable *pTab,             /* Table structure */
  int bPatchset,                  /* True for patchsets */
  SessionChange *pExist,          /* Existing change */
  int op2,                        /* Second change operation */
  int bIndirect,                  /* True if second change is indirect */
  u8 *aRec,                       /* Second change record */
  int nRec,                       /* Number of bytes in aRec */
  SessionChange **ppNew           /* OUT: Merged change */
){
  SessionChange *pNew = 0;

  if( !pExist ){
    pNew = (SessionChange *)sqlite3_malloc(sizeof(SessionChange) + nRec);
    if( !pNew ){
      return SQLITE_NOMEM;
    }
    memset(pNew, 0, sizeof(SessionChange));
    pNew->op = op2;
    pNew->bIndirect = bIndirect;
    pNew->nRecord = nRec;
    pNew->aRecord = (u8*)&pNew[1];
    memcpy(pNew->aRecord, aRec, nRec);
  }else{
    int op1 = pExist->op;

    /* 
    **   op1=INSERT, op2=INSERT      ->      Unsupported. Discard op2.
    **   op1=INSERT, op2=UPDATE      ->      INSERT.
    **   op1=INSERT, op2=DELETE      ->      (none)
    **
    **   op1=UPDATE, op2=INSERT      ->      Unsupported. Discard op2.
    **   op1=UPDATE, op2=UPDATE      ->      UPDATE.
    **   op1=UPDATE, op2=DELETE      ->      DELETE.
    **
    **   op1=DELETE, op2=INSERT      ->      UPDATE.
    **   op1=DELETE, op2=UPDATE      ->      Unsupported. Discard op2.
    **   op1=DELETE, op2=DELETE      ->      Unsupported. Discard op2.
    */   
    if( (op1==SQLITE_INSERT && op2==SQLITE_INSERT)
     || (op1==SQLITE_UPDATE && op2==SQLITE_INSERT)
     || (op1==SQLITE_DELETE && op2==SQLITE_UPDATE)
     || (op1==SQLITE_DELETE && op2==SQLITE_DELETE)
    ){
      pNew = pExist;
    }else if( op1==SQLITE_INSERT && op2==SQLITE_DELETE ){
      sqlite3_free(pExist);
      assert( pNew==0 );
    }else{
      u8 *aExist = pExist->aRecord;
      int nByte;
      u8 *aCsr;

      /* Allocate a new SessionChange object. Ensure that the aRecord[]
      ** buffer of the new object is large enough to hold any record that
      ** may be generated by combining the input records.  */
      nByte = sizeof(SessionChange) + pExist->nRecord + nRec;
      pNew = (SessionChange *)sqlite3_malloc(nByte);
      if( !pNew ){
        sqlite3_free(pExist);
        return SQLITE_NOMEM;
      }
      memset(pNew, 0, sizeof(SessionChange));
      pNew->bIndirect = (bIndirect && pExist->bIndirect);
      aCsr = pNew->aRecord = (u8 *)&pNew[1];

      if( op1==SQLITE_INSERT ){             /* INSERT + UPDATE */
        u8 *a1 = aRec;
        assert( op2==SQLITE_UPDATE );
        pNew->op = SQLITE_INSERT;
        if( bPatchset==0 ) sessionSkipRecord(&a1, pTab->nCol);
        sessionMergeRecord(&aCsr, pTab->nCol, aExist, a1);
      }else if( op1==SQLITE_DELETE ){       /* DELETE + INSERT */
        assert( op2==SQLITE_INSERT );
        pNew->op = SQLITE_UPDATE;
        if( bPatchset ){
          memcpy(aCsr, aRec, nRec);
          aCsr += nRec;
        }else{
          if( 0==sessionMergeUpdate(&aCsr, pTab, bPatchset, aExist, 0,aRec,0) ){
            sqlite3_free(pNew);
            pNew = 0;
          }
        }
      }else if( op2==SQLITE_UPDATE ){       /* UPDATE + UPDATE */
        u8 *a1 = aExist;
        u8 *a2 = aRec;
        assert( op1==SQLITE_UPDATE );
        if( bPatchset==0 ){
          sessionSkipRecord(&a1, pTab->nCol);
          sessionSkipRecord(&a2, pTab->nCol);
        }
        pNew->op = SQLITE_UPDATE;
        if( 0==sessionMergeUpdate(&aCsr, pTab, bPatchset, aRec, aExist,a1,a2) ){
          sqlite3_free(pNew);
          pNew = 0;
        }
      }else{                                /* UPDATE + DELETE */
        assert( op1==SQLITE_UPDATE && op2==SQLITE_DELETE );
        pNew->op = SQLITE_DELETE;
        if( bPatchset ){
          memcpy(aCsr, aRec, nRec);
          aCsr += nRec;
        }else{
          sessionMergeRecord(&aCsr, pTab->nCol, aRec, aExist);
        }
      }

      if( pNew ){
        pNew->nRecord = (int)(aCsr - pNew->aRecord);
      }
      sqlite3_free(pExist);
    }
  }

  *ppNew = pNew;
  return SQLITE_OK;
}

/*
** Add all changes in the changeset passed via the first two arguments to
** hash tables.
*/
static int sessionChangesetToHash(
  sqlite3_changeset_iter *pIter,   /* Iterator to read from */
  SessionTable **ppTabList        /* IN/OUT: List of table objects */
){
  u8 *aRec;
  int nRec;
  int rc = SQLITE_OK;
  SessionTable *pTab = 0;

  while( SQLITE_ROW==sessionChangesetNext(pIter, &aRec, &nRec) ){
    const char *zNew;
    int nCol;
    int op;
    int iHash;
    int bIndirect;
    SessionChange *pChange;
    SessionChange *pExist = 0;
    SessionChange **pp;

    sqlite3changeset_op(pIter, &zNew, &nCol, &op, &bIndirect);
    if( !pTab || sqlite3_stricmp(zNew, pTab->zName) ){
      /* Search the list for a matching table */
      int nNew = (int)strlen(zNew);
      u8 *abPK;

      sqlite3changeset_pk(pIter, &abPK, 0);
      for(pTab = *ppTabList; pTab; pTab=pTab->pNext){
        if( 0==sqlite3_strnicmp(pTab->zName, zNew, nNew+1) ) break;
      }
      if( !pTab ){
        pTab = sqlite3_malloc(sizeof(SessionTable) + nCol + nNew+1);
        if( !pTab ){
          rc = SQLITE_NOMEM;
          break;
        }
        memset(pTab, 0, sizeof(SessionTable));
        pTab->pNext = *ppTabList;
        pTab->nCol = nCol;
        pTab->abPK = (u8*)&pTab[1];
        memcpy(pTab->abPK, abPK, nCol);
        pTab->zName = (char*)&pTab->abPK[nCol];
        memcpy(pTab->zName, zNew, nNew+1);
        *ppTabList = pTab;
      }else if( pTab->nCol!=nCol || memcmp(pTab->abPK, abPK, nCol) ){
        rc = SQLITE_SCHEMA;
        break;
      }
    }

    if( sessionGrowHash(pIter->bPatchset, pTab) ){
      rc = SQLITE_NOMEM;
      break;
    }
    iHash = sessionChangeHash(
        pTab, (pIter->bPatchset && op==SQLITE_DELETE), aRec, pTab->nChange
    );

    /* Search for existing entry. If found, remove it from the hash table. 
    ** Code below may link it back in.
    */
    for(pp=&pTab->apChange[iHash]; *pp; pp=&(*pp)->pNext){
      int bPkOnly1 = 0;
      int bPkOnly2 = 0;
      if( pIter->bPatchset ){
        bPkOnly1 = (*pp)->op==SQLITE_DELETE;
        bPkOnly2 = op==SQLITE_DELETE;
      }
      if( sessionChangeEqual(pTab, bPkOnly1, (*pp)->aRecord, bPkOnly2, aRec) ){
        pExist = *pp;
        *pp = (*pp)->pNext;
        pTab->nEntry--;
        break;
      }
    }

    rc = sessionChangeMerge(pTab, 
        pIter->bPatchset, pExist, op, bIndirect, aRec, nRec, &pChange
    );
    if( rc ) break;
    if( pChange ){
      pChange->pNext = pTab->apChange[iHash];
      pTab->apChange[iHash] = pChange;
      pTab->nEntry++;
    }
  }

  if( rc==SQLITE_OK ) rc = pIter->rc;
  return rc;
}
  
/* 
** 1. Iterate through the left-hand changeset. Add an entry to a table
**    specific hash table for each change in the changeset. The hash table
**    key is the PK of the row affected by the change.
**
** 2. Then interate through the right-hand changeset. Attempt to add an 
**    entry to a hash table for each component change. If a change already 
**    exists with the same PK values, combine the two into a single change.
**
** 3. Write an output changeset based on the contents of the hash table.
*/
int sessionChangesetConcat(
  sqlite3_changeset_iter *pLeft,
  sqlite3_changeset_iter *pRight,
  int (*xOutput)(void *pOut, const void *pData, int nData),
  void *pOut,
  int *pnOut,
  void **ppOut
){
  SessionTable *pList = 0;        /* List of SessionTable objects */
  int rc;                         /* Return code */
  int bPatch;                     /* True for a patchset */
  SessionTable *pTab;
  SessionBuffer buf = {0, 0, 0};

  assert( xOutput==0 || (ppOut==0 && pnOut==0) );

  assert( pLeft->zTab==0 && pRight->zTab==0 );
  rc = sessionChangesetToHash(pLeft, &pList);
  assert( pLeft->zTab || pList==0 );
  if( rc==SQLITE_OK ){
    rc = sessionChangesetToHash(pRight, &pList);
  }
  bPatch = pLeft->bPatchset || pRight->bPatchset;

  if( pLeft->zTab && pRight->zTab && pLeft->bPatchset!=pRight->bPatchset ){
    rc = SQLITE_ERROR;
  }

  /* Create the serialized output changeset based on the contents of the
  ** hash tables attached to the SessionTable objects in list pList. 
  */
  for(pTab=pList; rc==SQLITE_OK && pTab; pTab=pTab->pNext){
    int i;
    if( pTab->nEntry==0 ) continue;

    sessionAppendTableHdr(&buf, bPatch, pTab, &rc);
    for(i=0; i<pTab->nChange; i++){
      SessionChange *p;
      for(p=pTab->apChange[i]; p; p=p->pNext){
        sessionAppendByte(&buf, p->op, &rc);
        sessionAppendByte(&buf, p->bIndirect, &rc);
        sessionAppendBlob(&buf, p->aRecord, p->nRecord, &rc);
      }
    }

    if( rc==SQLITE_OK && xOutput && buf.nBuf>=SESSIONS_STRM_CHUNK_SIZE ){
      rc = xOutput(pOut, buf.aBuf, buf.nBuf);
      buf.nBuf = 0;
    }
  }

  if( rc==SQLITE_OK ){
    if( xOutput ){
      if( buf.nBuf>0 ) rc = xOutput(pOut, buf.aBuf, buf.nBuf);
    }else{
      *ppOut = buf.aBuf;
      *pnOut = buf.nBuf;
      buf.aBuf = 0;
    }
  }
  sqlite3_free(buf.aBuf);

  sessionDeleteTable(pList);
  return rc;
}

/* 
** Combine two changesets together.
*/
SQLITE_API int sqlite3changeset_concat(
  int nLeft,                      /* Number of bytes in lhs input */
  void *pLeft,                    /* Lhs input changeset */
  int nRight                      /* Number of bytes in rhs input */,
  void *pRight,                   /* Rhs input changeset */
  int *pnOut,                     /* OUT: Number of bytes in output changeset */
  void **ppOut                    /* OUT: changeset (left <concat> right) */
){
  sqlite3_changeset_iter *pIter1 = 0;
  sqlite3_changeset_iter *pIter2 = 0;
  int rc;

  *pnOut = 0;
  *ppOut = 0;
  rc = sqlite3changeset_start(&pIter1, nLeft, pLeft);
  if( rc==SQLITE_OK ){
    rc = sqlite3changeset_start(&pIter2, nRight, pRight);
  }
  if( rc==SQLITE_OK ){
    rc = sessionChangesetConcat(pIter1, pIter2, 0, 0, pnOut, ppOut);
  }

  sqlite3changeset_finalize(pIter1);
  sqlite3changeset_finalize(pIter2);
  return rc;
}

/*
** Streaming version of sqlite3changeset_concat().
*/
SQLITE_API int sqlite3changeset_concat_strm(
  int (*xInputA)(void *pIn, void *pData, int *pnData),
  void *pInA,
  int (*xInputB)(void *pIn, void *pData, int *pnData),
  void *pInB,
  int (*xOutput)(void *pOut, const void *pData, int nData),
  void *pOut
){
  sqlite3_changeset_iter *pIter1 = 0;
  sqlite3_changeset_iter *pIter2 = 0;
  int rc;

  rc = sqlite3changeset_start_strm(&pIter1, xInputA, pInA);
  if( rc==SQLITE_OK ){
    rc = sqlite3changeset_start_strm(&pIter2, xInputB, pInB);
  }
  if( rc==SQLITE_OK ){
    rc = sessionChangesetConcat(pIter1, pIter2, xOutput, pOut, 0, 0);
  }

  sqlite3changeset_finalize(pIter1);
  sqlite3changeset_finalize(pIter2);
  return rc;
}

#endif /* SQLITE_ENABLE_SESSION && SQLITE_ENABLE_PREUPDATE_HOOK */

/************** End of sqlite3session.c **************************************/
