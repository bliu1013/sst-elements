// Copyright 2009-2015 Sandia Corporation. Under the terms
// of Contract DE-AC04-94AL85000 with Sandia Corporation, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2015, Sandia Corporation
// All rights reserved.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

/*
 * File:   cacheController.h
 */

#ifndef _CACHECONTROLLER_H_
#define _CACHECONTROLLER_H_

#include <boost/assert.hpp>
#include <queue>
#include <map>

#include <sst/core/event.h>
#include <sst/core/sst_types.h>
#include <sst/core/component.h>
#include <sst/core/link.h>
#include <sst/core/timeConverter.h>
#include <sst/core/output.h>

#include "../cacheArray.h"
#include "../replacementManager.h"
#include "coherenceControllers.h"
#include "util.h"
#include "../cacheListener.h"
#include <boost/assert.hpp>
#include <string>
#include <sstream>

#define assert_msg BOOST_ASSERT_MSG

namespace SST { namespace MemHierarchy {

using namespace std;


class Sieve : public SST::Component {
public:

    using CacheLine = CacheArray::CacheLine;
    using CCLine = TopCacheController::CCLine;
    using uint = unsigned int;
    using uint64 = uint64_t;

    SST::link cpu_link;
    virtual void init(unsigned int);
    virtual void finish(void);
    
    /** Creates cache componennt */
    static Sieve* cacheFactory(SST::ComponentId_t id, SST::Params& params);
    
    /** Computes the 'Base Address' of the requests.  The base address point the first address of the cache line */
    Addr toBaseAddr(Addr addr){
        Addr baseAddr = (addr) & ~(cf_.cacheArray_->getLineSize() - 1);  //Remove the block offset bits
        return baseAddr;
    }
    
private:
    struct CacheConfig;
    
    /** Constructor for Sieve Component */
    Sieve(ComponentId_t _id, Params &_params, CacheConfig _config);
    
    /** Handler for incoming link events.  */
    void processEvent(MemEvent* event);
    
    /** Function processes incomming access requests from the CPU */
    void processCacheRequest(MemEvent *event, Command cmd, Addr baseAddr);
    void processCacheReplacement(MemEvent *event, Command cmd, Addr baseAddr);

    /** Find replacement for the current request. */
    inline void allocateCacheLine(MemEvent *event, Addr baseAddr, int& lineIndex);

    /** Depending on the replacement policy and cache array type, this function appropriately
        searches for an LRU replacement candidate */
    inline CacheLine* findReplacementCacheLine(Addr baseAddr);

    /** At this point, cache line has been evicted or is not valid. 
        This function replaces cache line with the info/addr of the current request */
    inline void replaceCacheLine(int replacementCacheLineIndex, int& newCacheLineIndex, Addr newBaseAddr);
    
    /** Check if there a cache miss */
    inline bool isCacheMiss(int lineIndex);
    
    /** Find cache line by line index */
    inline CacheLine* getCacheLine(int lineIndex);
    
    /** Check whether this request will hit or miss in the cache - including correct coherence permission */
    int isCacheHit(MemEvent* _event, Command _cmd, Addr _baseAddr);
    
    /** Find out if number is a power of 2 */
    bool isPowerOfTwo(uint x){ return (x & (x - 1)) == 0; }
    
    /** Timestamp getter */
    uint64 getTimestamp(){ return timestamp_; }

    struct CacheConfig{
        CacheArray* cacheArray_;
        Output* dbg_;
        ReplacementMgr* rm_;
        uint numLines_;
        uint lineSize_;
    };
    
    CacheConfig             cf_;
    CacheListener*          listener_;
    Output*                 d_;
    Output*                 d2_;
    uint64                  timestamp_;
    std::map<MemEvent*,uint64> startTimeList;
    bool                    DEBUG_ALL;
    Addr                    DEBUG_ADDR;
    
 };

/*  Implementation Details
 
    The Sieve class serves as the main cache controller.  It is in charge or handling incoming
    SST-based events (cacheEventProcessing.cc) and forwarding the requests to the other system's
    subcomponents:
        - Cache Array:  Class in charge keeping track of all the cache lines in the cache.  The 
        Cache Line inner class stores the data and state related to a particular cache line.
 
        - Replacement Manager:  Class handles work related to the replacement policy of the cache.  
        Similar to Cache Array, this class is OO-based so different replacement policies are simply
        subclasses of the main based abstrac class (ReplacementMgr), therefore they need to implement 
        certain functions in order for them to work properly. Implemented policies are: least-recently-used (lru), 
        least-frequently-used (lfu), most-recently-used (mru), random, and not-most-recently-used (nmru).
 
        - Hash:  Class implements common hashing functions.  These functions are used by the Cache Array
        class.  For instance, a typical set associative array uses a simple hash function, whereas
        skewed associate and ZCaches use more advanced hashing functions.
 
 
    Key notes:
 
        - Class member variables have a suffix "_", while function parameters have it as a preffix.

*/
#endif