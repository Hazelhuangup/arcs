#ifndef _SHARED_BARCODE_MAP_H_
#define _SHARED_BARCODE_MAP_H_ 1

#include "DataStructures/BarcodeToSegment.h"
#include "DataStructures/Segment.h"

#include <google/sparse_hash_map>
#include <iterator>
#include <unordered_map>

#if HAVE_GOOGLE_SPARSE_HASH_MAP

#	include <google/sparse_hash_map>

	/** a contig ID and shared barcode count */
	typedef google::sparse_hash_map<ContigIndex, BarcodeCount> ContigToCount;
	/** maps contig => contigs with shared barcodes */
	typedef google::sparse_hash_map<ContigIndex, ContigToCount> SharedBarcodeMap;

#else

#	include <unordered_map>

	/** a contig ID and shared barcode count */
	typedef std::unordered_map<ContigIndex, BarcodeCount> ContigToCount;
	/** maps contig => contigs with shared barcodes */
	typedef std::unordered_map<ContigIndex, ContigToCount> SharedBarcodeMap;

#endif

typedef typename ContigToCount::const_iterator ContigToCountConstIt;
typedef typename ContigToCount::iterator ContigToCountIt;
typedef typename SharedBarcodeMap::const_iterator SharedBarcodeMapConstIt;
typedef typename SharedBarcodeMap::iterator SharedBarcodeMapIt;

static inline void buildSharedBarcodeMap(
	const SegmentToBarcode& segmentToBarcode,
	unsigned minSharedBarcodes,
	SharedBarcodeMap& sharedBarcodeMap)
{
	/* build temporary barcode => segment map */

	BarcodeToSegment barcodeToSegment;
	buildBarcodeToSegment(segmentToBarcode, barcodeToSegment);

	/* tally shared barcodes for contig pairs */

	for (BarcodeToSegmentConstIt barcodeIt = barcodeToSegment.begin();
		barcodeIt != barcodeToSegment.end(); ++barcodeIt)
	{
		const SegmentList& segments = *barcodeIt;
		for (SegmentListConstIt it1 = segments.begin();
			 it1 != segments.end(); ++it1)
		{
			for (SegmentListConstIt it2 = segments.begin();
				 it2 != segments.end(); ++it2)
			{
				if (it1->first == it2->first)
					continue;

				sharedBarcodeMap[it1->first][it2->first]++;
			}
		}
	}

	/* remove candidates with insufficient shared barcodes */

	for (SharedBarcodeMapIt it1 = sharedBarcodeMap.begin();
		it1 != sharedBarcodeMap.end(); ++it1)
	{
		ContigToCount& contigToCount = it1->second;
        contigToCount.set_deleted_key(std::numeric_limits<ContigIndex>::max());

		for (ContigToCountIt it2 = contigToCount.begin();
			it2 != contigToCount.end(); ++it2)
		{
			if (it2->second < minSharedBarcodes)
				contigToCount.erase(it2);
		}

        /* force google::sparse_hash_map to perform compaction of deleted keys */
        contigToCount.resize(0);
	}

}

#endif