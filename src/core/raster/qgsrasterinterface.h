/***************************************************************************
    qgsrasterface.h - Internal raster processing modules interface
     --------------------------------------
    Date                 : Jun 21, 2012
    Copyright            : (C) 2012 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERINTERFACE_H
#define QGSRASTERINTERFACE_H

#include "qgis_core.h"
#include <limits>

#include <QCoreApplication> // for tr()
#include <QImage>

#include "qgsfeedback.h"
#include "qgsrasterbandstats.h"
#include "qgsrasterblock.h"
#include "qgsrasterhistogram.h"
#include "qgsrectangle.h"

/** \ingroup core
 * Feedback object tailored for raster block reading.
 *
 * @note added in QGIS 3.0
 */
class CORE_EXPORT QgsRasterBlockFeedback : public QgsFeedback
{
  public:
    //! Construct a new raster block feedback object
    QgsRasterBlockFeedback( QObject* parent = nullptr ) : QgsFeedback( parent ), mPreviewOnly( false ), mRenderPartialOutput( false ) {}

    //! May be emitted by raster data provider to indicate that some partial data are available
    //! and a new preview image may be produced
    virtual void onNewData() {}

    //! Whether the raster provider should return only data that are already available
    //! without waiting for full result. By default this flag is not enabled.
    //! @see setPreviewOnly()
    bool isPreviewOnly() const { return mPreviewOnly; }
    //! set flag whether the block request is for preview purposes only
    //! @see isPreviewOnly()
    void setPreviewOnly( bool preview ) { mPreviewOnly = preview; }

    //! Whether our painter is drawing to a temporary image used just by this layer
    //! @see setRenderPartialOutput()
    bool renderPartialOutput() const { return mRenderPartialOutput; }
    //! Set whether our painter is drawing to a temporary image used just by this layer
    //! @see renderPartialOutput()
    void setRenderPartialOutput( bool enable ) { mRenderPartialOutput = enable; }

  private:
    //! Whether the raster provider should return only data that are already available
    //! without waiting for full result
    bool mPreviewOnly;

    //! Whether our painter is drawing to a temporary image used just by this layer
    bool mRenderPartialOutput;
};


/** \ingroup core
 * Base class for processing filters like renderers, reprojector, resampler etc.
 */
class CORE_EXPORT QgsRasterInterface
{
    Q_DECLARE_TR_FUNCTIONS( QgsRasterInterface )

  public:
    //! If you add to this, please also add to capabilitiesString()
    enum Capability
    {
      NoCapabilities   = 0,
      Size             = 1 << 1, // original data source size (and thus resolution) is known, it is not always available, for example for WMS
      Create           = 1 << 2, // create new datasets
      Remove           = 1 << 3, // delete datasets
      BuildPyramids    = 1 << 4, // supports building of pyramids (overviews)
      Identify         = 1 << 5, // at least one identify format supported
      IdentifyValue    = 1 << 6, // numerical values
      IdentifyText     = 1 << 7, // WMS text
      IdentifyHtml     = 1 << 8, // WMS HTML
      IdentifyFeature  = 1 << 9, // WMS GML -> feature
    };

    QgsRasterInterface( QgsRasterInterface * input = nullptr );

    virtual ~QgsRasterInterface() = default;

    //! Clone itself, create deep copy
    virtual QgsRasterInterface *clone() const = 0;

    //! Returns a bitmask containing the supported capabilities
    virtual int capabilities() const
    {
      return QgsRasterInterface::NoCapabilities;
    }

    /**
     *  Returns the above in friendly format.
     */
    QString capabilitiesString() const;

    //! Returns data type for the band specified by number
    virtual Qgis::DataType dataType( int bandNo ) const = 0;

    /** Returns source data type for the band specified by number,
     *  source data type may be shorter than dataType */
    virtual Qgis::DataType sourceDataType( int bandNo ) const { return mInput ? mInput->sourceDataType( bandNo ) : Qgis::UnknownDataType; }

    /**
     * Get the extent of the interface.
     * @return QgsRectangle containing the extent of the layer
     */
    virtual QgsRectangle extent() const { return mInput ? mInput->extent() : QgsRectangle(); }

    int dataTypeSize( int bandNo ) { return QgsRasterBlock::typeSize( dataType( bandNo ) ); }

    //! Get number of bands
    virtual int bandCount() const = 0;

    //! Get block size
    virtual int xBlockSize() const { return mInput ? mInput->xBlockSize() : 0; }
    virtual int yBlockSize() const { return mInput ? mInput->yBlockSize() : 0; }

    //! Get raster size
    virtual int xSize() const { return mInput ? mInput->xSize() : 0; }
    virtual int ySize() const { return mInput ? mInput->ySize() : 0; }

    //! \brief helper function to create zero padded band names
    virtual QString generateBandName( int theBandNumber ) const
    {
      return tr( "Band" ) + QStringLiteral( " %1" ) .arg( theBandNumber, 1 + static_cast< int >( log10( static_cast< double >( bandCount() ) ) ), 10, QChar( '0' ) );
    }

    /** Read block of data using given extent and size.
     *  Returns pointer to data.
     *  Caller is responsible to free the memory returned.
     * @param bandNo band number
     * @param extent extent of block
     * @param width pixel width of block
     * @param height pixel height of block
     * @param feedback optional raster feedback object for cancelation/preview. Added in QGIS 3.0.
     */
    virtual QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback* feedback = nullptr ) = 0;

    /** Set input.
      * Returns true if set correctly, false if cannot use that input */
    virtual bool setInput( QgsRasterInterface* input ) { mInput = input; return true; }

    //! Current input
    virtual QgsRasterInterface * input() const { return mInput; }

    //! Is on/off
    virtual bool on() const { return mOn; }

    //! Set on/off
    virtual void setOn( bool on ) { mOn = on; }

    /** Get source / raw input, the first in pipe, usually provider.
     *  It may be used to get info about original data, e.g. resolution to decide
     *  resampling etc.
     */
    virtual const QgsRasterInterface *sourceInput() const
    {
      QgsDebugMsgLevel( "Entered", 4 );
      return mInput ? mInput->sourceInput() : this;
    }

    /** Get source / raw input, the first in pipe, usually provider.
     *  It may be used to get info about original data, e.g. resolution to decide
     *  resampling etc.
     */
    virtual QgsRasterInterface * sourceInput()
    {
      QgsDebugMsgLevel( "Entered", 4 );
      return mInput ? mInput->sourceInput() : this;
    }

    /** \brief Get band statistics.
     * @param theBandNo The band (number).
     * @param theStats Requested statistics
     * @param theExtent Extent used to calc statistics, if empty, whole raster extent is used.
     * @param theSampleSize Approximate number of cells in sample. If 0, all cells (whole raster will be used). If raster does not have exact size (WCS without exact size for example), provider decides size of sample.
     * @return Band statistics.
     */
    virtual QgsRasterBandStats bandStatistics( int theBandNo,
        int theStats = QgsRasterBandStats::All,
        const QgsRectangle & theExtent = QgsRectangle(),
        int theSampleSize = 0 );

    /** \brief Returns true if histogram is available (cached, already calculated).     *   The parameters are the same as in bandStatistics()
     * @return true if statistics are available (ready to use)
     */
    virtual bool hasStatistics( int theBandNo,
                                int theStats = QgsRasterBandStats::All,
                                const QgsRectangle & theExtent = QgsRectangle(),
                                int theSampleSize = 0 );

    /** \brief Get histogram. Histograms are cached in providers.
     * @param theBandNo The band (number).
     * @param theBinCount Number of bins (intervals,buckets). If 0, the number of bins is decided automatically according to data type, raster size etc.
     * @param theMinimum Minimum value, if NaN, raster minimum value will be used.
     * @param theMaximum Maximum value, if NaN, raster minimum value will be used.
     * @param theExtent Extent used to calc histogram, if empty, whole raster extent is used.
     * @param theSampleSize Approximate number of cells in sample. If 0, all cells (whole raster will be used). If raster does not have exact size (WCS without exact size for example), provider decides size of sample.
     * @param theIncludeOutOfRange include out of range values
     * @return Vector of non NULL cell counts for each bin.
     * @note theBinCount, theMinimum and theMaximum not optional in python bindings
     */
    virtual QgsRasterHistogram histogram( int theBandNo,
                                          int theBinCount = 0,
                                          double theMinimum = std::numeric_limits<double>::quiet_NaN(),
                                          double theMaximum = std::numeric_limits<double>::quiet_NaN(),
                                          const QgsRectangle & theExtent = QgsRectangle(),
                                          int theSampleSize = 0,
                                          bool theIncludeOutOfRange = false );

    /** \brief Returns true if histogram is available (cached, already calculated), the parameters are the same as in histogram()
     * @note theBinCount, theMinimum and theMaximum not optional in python bindings
     */
    virtual bool hasHistogram( int theBandNo,
                               int theBinCount,
                               double theMinimum = std::numeric_limits<double>::quiet_NaN(),
                               double theMaximum = std::numeric_limits<double>::quiet_NaN(),
                               const QgsRectangle & theExtent = QgsRectangle(),
                               int theSampleSize = 0,
                               bool theIncludeOutOfRange = false );

    /** \brief Find values for cumulative pixel count cut.
     * @param theBandNo The band (number).
     * @param theLowerCount The lower count as fraction of 1, e.g. 0.02 = 2%
     * @param theUpperCount The upper count as fraction of 1, e.g. 0.98 = 98%
     * @param theLowerValue Location into which the lower value will be set.
     * @param theUpperValue  Location into which the upper value will be set.
     * @param theExtent Extent used to calc histogram, if empty, whole raster extent is used.
     * @param theSampleSize Approximate number of cells in sample. If 0, all cells (whole raster will be used). If raster does not have exact size (WCS without exact size for example), provider decides size of sample.
     */
    virtual void cumulativeCut( int theBandNo,
                                double theLowerCount,
                                double theUpperCount,
                                double &theLowerValue,
                                double &theUpperValue,
                                const QgsRectangle & theExtent = QgsRectangle(),
                                int theSampleSize = 0 );

    //! Write base class members to xml.
    virtual void writeXml( QDomDocument& doc, QDomElement& parentElem ) const { Q_UNUSED( doc ); Q_UNUSED( parentElem ); }
    //! Sets base class members from xml. Usually called from create() methods of subclasses
    virtual void readXml( const QDomElement& filterElem ) { Q_UNUSED( filterElem ); }

  protected:
    // QgsRasterInterface used as input
    QgsRasterInterface* mInput;

    //! \brief List  of cached statistics, all bands mixed
    QList <QgsRasterBandStats> mStatistics;

    //! \brief List  of cached histograms, all bands mixed
    QList <QgsRasterHistogram> mHistograms;

    // On/off state, if off, it does not do anything, replicates input
    bool mOn;

    /** Fill in histogram defaults if not specified
     * @note theBinCount, theMinimum and theMaximum not optional in python bindings
     */
    void initHistogram( QgsRasterHistogram &theHistogram, int theBandNo,
                        int theBinCount = 0,
                        double theMinimum = std::numeric_limits<double>::quiet_NaN(),
                        double theMaximum = std::numeric_limits<double>::quiet_NaN(),
                        const QgsRectangle & theExtent = QgsRectangle(),
                        int theSampleSize = 0,
                        bool theIncludeOutOfRange = false );

    //! Fill in statistics defaults if not specified
    void initStatistics( QgsRasterBandStats &theStatistics, int theBandNo,
                         int theStats = QgsRasterBandStats::All,
                         const QgsRectangle & theExtent = QgsRectangle(),
                         int theBinCount = 0 );

  private:
    Q_DISABLE_COPY( QgsRasterInterface )   // there is clone() for copying
};

#endif


