/*
 * This file is part of the PeerDrive Qt4 library.
 * Copyright (C) 2012  Jan Klötzke <jan DOT kloetzke AT freenet DOT de>
 *
 * PeerDrive is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * PeerDrive is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with PeerDrive. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef _PEERDRIVE_H_
#define _PEERDRIVE_H_

#include <QList>
#include <QString>
#include <QDateTime>
#include <QMap>
#include <QByteArray>

#include <string>

namespace PeerDrive {
	class Link;
}

uint qHash(const PeerDrive::Link&);

namespace PeerDrive {

class EnumCnf_Store;

class DId {
public:
	DId() { };
	DId(const QByteArray &ba) { id = std::string(ba.constData(), ba.size()); }
	DId(const std::string &str) : id(str) { };
	std::string toStdString() const { return id; };
	QByteArray toByteArray() const { return QByteArray(id.c_str(), id.size()); };
	bool operator== (const DId &other) const { return id == other.id; };
	bool operator!= (const DId &other) const { return id != other.id; };
	bool operator< (const DId &other) const { return id < other.id; }

private:
	std::string id;
};

class RId {
public:
	RId() { };
	RId(const std::string &str) : id(str) { };
	RId(const QByteArray &ba) { id = std::string(ba.constData(), ba.size()); }
	std::string toStdString() const { return id; };
	QByteArray toByteArray() const { return QByteArray(id.c_str(), id.size()); };
	bool operator== (const RId &other) const { return id == other.id; };
	bool operator!= (const RId &other) const { return id != other.id; };
	bool operator< (const RId &other) const { return id < other.id; }
private:
	std::string id;
};

class PId {
public:
	PId() { };
	PId(const std::string &str) : id(str) { };
	PId(const QByteArray &ba) { id = std::string(ba.constData(), ba.size()); }
	std::string toStdString() const { return id; };
	bool operator== (const PId &other) const { return id == other.id; };
	bool operator!= (const PId &other) const { return id != other.id; };
	bool operator< (const PId &other) const { return id < other.id; }
private:
	std::string id;
};

class Part {
public:
	Part(const std::string &str) : part(str) { };
	std::string toStdString() const { return part; };

	bool operator== (const Part &other) const { return part == other.part; };
	bool operator!= (const Part &other) const { return part != other.part; };
	bool operator< (const Part &other) const { return part < other.part; }

	static const Part FILE;
	static const Part META;
	static const Part PDSD;

private:
	std::string part;
};

/**
 * Represents three possible link classes:
 *  - DocHeadLinks
 *  - DocPreLinks
 *  - RevLinks
 */
class Link {
public:
	Link();
	Link(const DId &store, const RId &rev);
	Link(const DId &store, const DId &doc, bool update = true);
	Link(const DId &store, const DId &doc, const RId &rev, bool isPreRev = false);

	bool operator== (const Link &other) const;
	bool operator!= (const Link &other) const { return !(*this == other); }
	bool operator< (const Link &other) const;

	bool update();
	bool update(DId &newStore);

	bool isValid() const;
	bool isRevLink() const;
	bool isDocLink() const;
	bool isDocHeadLink() const;
	bool isDocPreRevLink() const;

	DId store() const;
	RId rev() const;
	DId doc() const;

	QString getOsPath() const;

protected:
	enum State {
		INVALID,
		DOC_NOT_FOUND,
		DOC_PRE_REV,
		DOC_HEAD,
		REV
	};

	State m_state;
	DId m_store;
	RId m_rev;
	DId m_doc;

	friend uint ::qHash(const Link&);
};

class Watch : public QObject {
	Q_OBJECT

public:
	Watch();
	virtual ~Watch();

	void watch(const Link &item);
	void clear();

	enum Distribution {
		APPEARED,
		REPLICATED,
		DIMINISHED,
		DISAPPEARED
	};

signals:
	void modified();
	void appeared();
	void replicated();
	void diminished();
	void disappeared();
	void distributionChanged(Distribution how);

private:
	void dispatch(int event);
	friend class Connection;

	bool m_watching;
	Link m_link;
};

class Mounts {
public:
	struct Store {
		DId sid;
		QString src;
		QString type;
		QString label;
		QString options;
		bool isSystem;
	};

	Mounts();
	~Mounts();

	Store* sysStore() const;
	QList<Store*> regularStores() const;
	QList<Store*> allStores() const;
	Store* fromLabel(const QString &label) const;
	Store* fromSId(const DId &sid) const;

	static DId mount(int *result, const QString &src, const QString &label,
	                 const QString &type, const QString &options = QString(),
					 const QString &credentials = QString());
	static int unmount(const DId &sid);

private:
	Store *parse(const EnumCnf_Store &pb);
	Store *m_sysStore;
	QList<Store*> m_stores;
};


class RevInfo {
public:
	RevInfo();
	RevInfo(const RId &rid);
	RevInfo(const RId &rid, const QList<DId> &stores);
	RevInfo(const RevInfo &revInfo);
	~RevInfo();

	bool exists();
	int error();

	QList<DId> stores();
	int flags();
	quint64 size();
	quint64 size(const Part &part);
	PId hash(const Part &part);
	QList<Part> parts();
	QList<RId> parents();
	QDateTime mtime();
	QString type();
	QString creator();
	QString comment();

private:
	void fetch(const RId &rid, const QList<DId> *stores);

	bool m_exists;
	int m_error;
	quint32 m_flags;
	QList<RId> m_parents;
	QDateTime m_mtime;
	QString m_type;
	QString m_creator;
	QString m_comment;
	QMap<Part, quint64> m_partSizes;
	QMap<Part, PId> m_partHashes;
};

class DocInfo {
public:
	DocInfo();
	DocInfo(const DId &doc);
	DocInfo(const DId &doc, const DId &store);
	DocInfo(const DId &doc, const QList<DId> &stores);

	bool exists() const;
	bool exists(const DId &store) const;
	int error() const;

	QList<DId> stores() const;
	QList<DId> stores(const RId &rev) const;
	QList<Link> heads() const;
	Link head(const DId &store) const;
	QList<Link> preliminaryHeads() const;
	QList<Link> preliminaryHeads(const DId &store) const;

private:
	void fetch(const DId &doc, const QList<DId> &stores = QList<DId>());

	int m_error;
	DId m_doc;
	QMap<DId, QList<Link> > m_stores; // store -> revs ++ preRevs
	QMap<RId, QList<DId> > m_revs; // revs -> stores
	QMap<RId, QList<DId> > m_preRevs; // preRevs -> stores
};

/**
 *
 * The Document will track the revision of the document that was last used. That
 * means that the object is always pointing to the last assigned, read or committed
 * revision. If the document was changed by some other process the object will not
 *
 */
class Document {
public:
	Document();
	Document(const Link &link);
	~Document();

	static Document *fork(const Link &parent);
	static Document *create(const DId &store);

	const Link link() const;
	void setLink(const Link &link);
	int error() const;

	bool peek();
	bool update(const QString &creator = QString());
	bool resume(const QString &creator = QString());

	bool forget();
	bool remove();
	bool replicateTo(DId &store, QDateTime depth=QDateTime(), bool verbose=false);
	bool forward(const RId &toRev, const DId &srcStore,
		QDateTime depth=QDateTime(), bool verbose=false);

	qint64 pos(const Part &part) const;
	bool seek(const Part &part, qint64 pos);

	qint64 read(const Part &part, char *data, qint64 maxSize);
	qint64 read(const Part &part, QByteArray &data, qint64 maxSize);
	qint64 readAll(const Part &part, QByteArray &data);

	bool write(const Part &part, const char *data, qint64 size);
	bool write(const Part &part, const char *data);
	bool write(const Part &part, const QByteArray &data);
	bool writeAll(const Part &part, const char *data, qint64 size);
	bool writeAll(const Part &part, const char *data);
	bool writeAll(const Part &part, const QByteArray &data);
	bool resize(const Part &part, qint64 size);

	unsigned int flags() const;
	bool setFlags(unsigned int flags);
	QString type() const;
	bool setType(const QString &type);
	QList<Link> parents() const;
	bool merge(const Link &rev, QDateTime depth=QDateTime(), bool verbose=false);
	bool rebase(const RId &parent);

	void close();
	bool commit(const QString &comment = QString());
	bool suspend(const QString &comment = QString());

private:
	qint64 read(const Part &part, char *data, qint64 maxSize, qint64 off);

	bool m_open;
	unsigned int m_handle;
	int m_error;
	Link m_link;
	QMap<Part, qint64> m_pos;
	mutable QString m_type;
};

}

#endif
