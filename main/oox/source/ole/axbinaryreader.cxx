/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



#include "oox/ole/axbinaryreader.hxx"

#include "oox/ole/olehelper.hxx"

namespace oox {
namespace ole {

// ============================================================================

using ::rtl::OUString;

// ============================================================================

namespace {

const sal_uInt32 AX_STRING_SIZEMASK         = 0x7FFFFFFF;
const sal_uInt32 AX_STRING_COMPRESSED       = 0x80000000;

} // namespace

// ============================================================================

AxAlignedInputStream::AxAlignedInputStream( BinaryInputStream& rInStrm ) :
    BinaryStreamBase( false ),
    mpInStrm( &rInStrm ),
    mnStrmPos( 0 ),
    mnStrmSize( rInStrm.getRemaining() )
{
    mbEof = mbEof || rInStrm.isEof();
}

sal_Int64 AxAlignedInputStream::size() const
{
    return mpInStrm ? mnStrmSize : -1;
}

sal_Int64 AxAlignedInputStream::tell() const
{
    return mpInStrm ? mnStrmPos : -1;
}

void AxAlignedInputStream::seek( sal_Int64 nPos )
{
    mbEof = mbEof || (nPos < mnStrmPos);
    if( !mbEof )
        skip( static_cast< sal_Int32 >( nPos - mnStrmPos ) );
}

void AxAlignedInputStream::close()
{
    mpInStrm = 0;
    mbEof = true;
}

sal_Int32 AxAlignedInputStream::readData( StreamDataSequence& orData, sal_Int32 nBytes, size_t nAtomSize )
{
    sal_Int32 nReadSize = 0;
    if( !mbEof )
    {
        nReadSize = mpInStrm->readData( orData, nBytes, nAtomSize );
        mnStrmPos += nReadSize;
        mbEof = mpInStrm->isEof();
    }
    return nReadSize;
}

sal_Int32 AxAlignedInputStream::readMemory( void* opMem, sal_Int32 nBytes, size_t nAtomSize )
{
    sal_Int32 nReadSize = 0;
    if( !mbEof )
    {
        nReadSize = mpInStrm->readMemory( opMem, nBytes, nAtomSize );
        mnStrmPos += nReadSize;
        mbEof = mpInStrm->isEof();
    }
    return nReadSize;
}

void AxAlignedInputStream::skip( sal_Int32 nBytes, size_t nAtomSize )
{
    if( !mbEof )
    {
        mpInStrm->skip( nBytes, nAtomSize );
        mnStrmPos += nBytes;
        mbEof = mpInStrm->isEof();
    }
}

void AxAlignedInputStream::align( size_t nSize )
{
    skip( static_cast< sal_Int32 >( (nSize - (mnStrmPos % nSize)) % nSize ) );
}

// ============================================================================

AxFontData::AxFontData() :
    mnFontEffects( 0 ),
    mnFontHeight( 160 ),
    mnFontCharSet( WINDOWS_CHARSET_DEFAULT ),
    mnHorAlign( AX_FONTDATA_LEFT ),
    mbDblUnderline( false )
{
}

sal_Int16 AxFontData::getHeightPoints() const
{
    /*  MSO uses weird font sizes:
        1pt->30, 2pt->45, 3pt->60, 4pt->75, 5pt->105, 6pt->120, 7pt->135,
        8pt->165, 9pt->180, 10pt->195, 11pt->225, ... */
    return getLimitedValue< sal_Int16, sal_Int32 >( (mnFontHeight + 10) / 20, 1, SAL_MAX_INT16 );
}

void AxFontData::setHeightPoints( sal_Int16 nPoints )
{
    mnFontHeight = getLimitedValue< sal_Int32, sal_Int32 >( ((nPoints * 4 + 1) / 3) * 15, 30, 4294967 );
}

bool AxFontData::importBinaryModel( BinaryInputStream& rInStrm )
{
    AxBinaryPropertyReader aReader( rInStrm );
    aReader.readStringProperty( maFontName );
    aReader.readIntProperty< sal_uInt32 >( mnFontEffects );
    aReader.readIntProperty< sal_Int32 >( mnFontHeight );
    aReader.skipIntProperty< sal_Int32 >(); // font offset
    aReader.readIntProperty< sal_uInt8 >( mnFontCharSet );
    aReader.skipIntProperty< sal_uInt8 >(); // font pitch/family
    aReader.readIntProperty< sal_uInt8 >( mnHorAlign );
    aReader.skipIntProperty< sal_uInt16 >(); // font weight
    mbDblUnderline = false;
    return aReader.finalizeImport();
}

bool AxFontData::importStdFont( BinaryInputStream& rInStrm )
{
    StdFontInfo aFontInfo;
    if( OleHelper::importStdFont( aFontInfo, rInStrm, false ) )
    {
        maFontName = aFontInfo.maName;
        mnFontEffects = 0;
        setFlag( mnFontEffects, AX_FONTDATA_BOLD,      aFontInfo.mnWeight >= OLE_STDFONT_BOLD );
        setFlag( mnFontEffects, AX_FONTDATA_ITALIC,    getFlag( aFontInfo.mnFlags, OLE_STDFONT_ITALIC ) );
        setFlag( mnFontEffects, AX_FONTDATA_UNDERLINE, getFlag( aFontInfo.mnFlags, OLE_STDFONT_UNDERLINE ) );
        setFlag( mnFontEffects, AX_FONTDATA_STRIKEOUT, getFlag( aFontInfo.mnFlags,OLE_STDFONT_STRIKE ) );
        mbDblUnderline = false;
        // StdFont stores font height in 1/10,000 of points
        setHeightPoints( getLimitedValue< sal_Int16, sal_Int32 >( aFontInfo.mnHeight / 10000, 0, SAL_MAX_INT16 ) );
        mnFontCharSet = aFontInfo.mnCharSet;
        mnHorAlign = AX_FONTDATA_LEFT;
        return true;
    }
    return false;
}

bool AxFontData::importGuidAndFont( BinaryInputStream& rInStrm )
{
    OUString aGuid = OleHelper::importGuid( rInStrm );
    if( aGuid.equalsAscii( AX_GUID_CFONT ) )
        return importBinaryModel( rInStrm );
    if( aGuid.equalsAscii( OLE_GUID_STDFONT ) )
        return importStdFont( rInStrm );
    return false;
}

// ============================================================================

namespace {

bool lclReadString( AxAlignedInputStream& rInStrm, OUString& rValue, sal_uInt32 nSize, bool bArrayString )
{
    bool bCompressed = getFlag( nSize, AX_STRING_COMPRESSED );
    sal_uInt32 nBufSize = nSize & AX_STRING_SIZEMASK;
    // Unicode: simple strings store byte count, array strings store char count
    sal_Int32 nChars = static_cast< sal_Int32 >( nBufSize / ((bCompressed || bArrayString) ? 1 : 2) );
    bool bValidChars = nChars <= 65536;
    OSL_ENSURE( bValidChars, "lclReadString - string too long" );
    sal_Int64 nEndPos = rInStrm.tell() + nChars * (bCompressed ? 1 : 2);
    nChars = ::std::min< sal_Int32 >( nChars, 65536 );
    rValue = rInStrm.readCompressedUnicodeArray( nChars, bCompressed );
    rInStrm.seek( nEndPos );
    return bValidChars;
}

} // namespace

// ----------------------------------------------------------------------------

AxBinaryPropertyReader::ComplexProperty::~ComplexProperty()
{
}

bool AxBinaryPropertyReader::PairProperty::readProperty( AxAlignedInputStream& rInStrm )
{
    rInStrm >> mrPairData.first >> mrPairData.second;
    return true;
}

bool AxBinaryPropertyReader::StringProperty::readProperty( AxAlignedInputStream& rInStrm )
{
    return lclReadString( rInStrm, mrValue, mnSize, false );
}

bool AxBinaryPropertyReader::StringArrayProperty::readProperty( AxAlignedInputStream& rInStrm )
{
    sal_Int64 nEndPos = rInStrm.tell() + mnSize;
    while( rInStrm.tell() < nEndPos )
    {
        OUString aString;
        if( !lclReadString( rInStrm, aString, rInStrm.readuInt32(), true ) )
            return false;
        mrArray.push_back( aString );
        // every array string is aligned on 4 byte boundries
        rInStrm.align( 4 );
    }
    return true;
}

bool AxBinaryPropertyReader::GuidProperty::readProperty( AxAlignedInputStream& rInStrm )
{
    mrGuid = OleHelper::importGuid( rInStrm );
    return true;
}

bool AxBinaryPropertyReader::FontProperty::readProperty( AxAlignedInputStream& rInStrm )
{
    return mrFontData.importGuidAndFont( rInStrm );
}

bool AxBinaryPropertyReader::PictureProperty::readProperty( AxAlignedInputStream& rInStrm )
{
    return OleHelper::importStdPic( mrPicData, rInStrm, true );
}

// ----------------------------------------------------------------------------

AxBinaryPropertyReader::AxBinaryPropertyReader( BinaryInputStream& rInStrm, bool b64BitPropFlags ) :
    maInStrm( rInStrm ),
    mbValid( true )
{
    // version and size of property block
    maInStrm.skip( 2 );
    sal_uInt16 nBlockSize = maInStrm.readValue< sal_uInt16 >();
    mnPropsEnd = maInStrm.tell() + nBlockSize;
    // flagfield containing existing properties
    if( b64BitPropFlags )
        maInStrm >> mnPropFlags;
    else
        mnPropFlags = maInStrm.readuInt32();
    mnNextProp = 1;
}

void AxBinaryPropertyReader::readBoolProperty( bool& orbValue, bool bReverse )
{
    // there is no data, the boolean value is equivalent to the property flag itself
    orbValue = startNextProperty() != bReverse;
}

void AxBinaryPropertyReader::readPairProperty( AxPairData& orPairData )
{
    if( startNextProperty() )
        maLargeProps.push_back( ComplexPropVector::value_type( new PairProperty( orPairData ) ) );
}

void AxBinaryPropertyReader::readStringProperty( OUString& orValue )
{
    if( startNextProperty() )
    {
        sal_uInt32 nSize = maInStrm.readAligned< sal_uInt32 >();
        maLargeProps.push_back( ComplexPropVector::value_type( new StringProperty( orValue, nSize ) ) );
    }
}

void AxBinaryPropertyReader::readStringArrayProperty( AxStringArray& orArray )
{
    if( startNextProperty() )
    {
        sal_uInt32 nSize = maInStrm.readAligned< sal_uInt32 >();
        maLargeProps.push_back( ComplexPropVector::value_type( new StringArrayProperty( orArray, nSize ) ) );
    }
}

void AxBinaryPropertyReader::readGuidProperty( ::rtl::OUString& orGuid )
{
    if( startNextProperty() )
        maLargeProps.push_back( ComplexPropVector::value_type( new GuidProperty( orGuid ) ) );
}

void AxBinaryPropertyReader::readFontProperty( AxFontData& orFontData )
{
    if( startNextProperty() )
    {
        sal_Int16 nData = maInStrm.readAligned< sal_Int16 >();
        if( ensureValid( nData == -1 ) )
            maStreamProps.push_back( ComplexPropVector::value_type( new FontProperty( orFontData ) ) );
    }
}

void AxBinaryPropertyReader::readPictureProperty( StreamDataSequence& orPicData )
{
    if( startNextProperty() )
    {
        sal_Int16 nData = maInStrm.readAligned< sal_Int16 >();
        if( ensureValid( nData == -1 ) )
            maStreamProps.push_back( ComplexPropVector::value_type( new PictureProperty( orPicData ) ) );
    }
}

bool AxBinaryPropertyReader::finalizeImport()
{
    // read large properties
    maInStrm.align( 4 );
    if( ensureValid( mnPropFlags == 0 ) && !maLargeProps.empty() )
    {
        for( ComplexPropVector::iterator aIt = maLargeProps.begin(), aEnd = maLargeProps.end(); ensureValid() && (aIt != aEnd); ++aIt )
        {
            ensureValid( (*aIt)->readProperty( maInStrm ) );
            maInStrm.align( 4 );
        }
    }
    maInStrm.seek( mnPropsEnd );

    // read stream properties (no stream alignment between properties!)
    if( ensureValid() && !maStreamProps.empty() )
        for( ComplexPropVector::iterator aIt = maStreamProps.begin(), aEnd = maStreamProps.end(); ensureValid() && (aIt != aEnd); ++aIt )
            ensureValid( (*aIt)->readProperty( maInStrm ) );

    return mbValid;
}

bool AxBinaryPropertyReader::ensureValid( bool bCondition )
{
    mbValid = mbValid && bCondition && !maInStrm.isEof();
    return mbValid;
}

bool AxBinaryPropertyReader::startNextProperty()
{
    bool bHasProp = getFlag( mnPropFlags, mnNextProp );
    setFlag( mnPropFlags, mnNextProp, false );
    mnNextProp <<= 1;
    return ensureValid() && bHasProp;
}

// ============================================================================

} // namespace ole
} // namespace oox
