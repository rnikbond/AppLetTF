//----------------------------------------
#include "ChangesetCache.h"
//----------------------------------------

ChangesetCache::ChangesetCache() {
}
//----------------------------------------------------------------------------------------------------------

ChangesetCache::CacheStatuses ChangesetCache::status( const QString& azurePath ) const {

    if( !m_cache.contains(azurePath) ) {
        return NoInfo;
    }

    const TfsItemInfo& info = m_cache[azurePath];
    if( info.m_localChangeset != info.m_azureChangeset ) {
        return Outdated;
    }

    return Lastest;
}
//----------------------------------------------------------------------------------------------------------

void ChangesetCache::upsert( const TfsItemInfo& info ) {

    m_cache[info.m_azurePath] = info;
}
//----------------------------------------------------------------------------------------------------------

