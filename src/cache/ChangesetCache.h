//----------------------------------------
#ifndef CHANGESETCACHE_H
#define CHANGESETCACHE_H
//----------------------------------------
#include <QMap>
#include <QString>
//----------------------------------------
struct TfsItemInfo {
    QString m_azurePath;
    QString m_localPath;
    int     m_azureChangeset;
    int     m_localChangeset;
};
//----------------------------------------

class ChangesetCache {

    QMap<QString, TfsItemInfo> m_cache;

public:

    enum CacheStatuses {
        NoInfo   = 0,
        Outdated,
        Lastest
    };

public:

    ChangesetCache();

    CacheStatuses status( const QString& azurePath ) const;
    void upsert( const TfsItemInfo& info );
};
//----------------------------------------------------------------------------------------------------------

#endif // CHANGESETCACHE_H
