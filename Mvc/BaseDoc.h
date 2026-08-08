#pragma once

#include <QObject>
#include <QImage>

class BaseDoc : public QObject
{
	Q_OBJECT

public:
	BaseDoc(QObject *parent);
	virtual ~BaseDoc();

	virtual void process();
	virtual void stopProcess();
	virtual bool isValid();

	bool isImageLoaded();
	int imgWidth();
	int imgHeight();
	void updateViews();

signals:
	void imageUpdated(const QImage&);
	void dataUpdated();

public:
	const QImage& getImage() const { return m_image; }

protected:
	QImage m_image;

public:
	virtual void clear();
	virtual bool loadImage(const QString& fname);
	virtual void setImage(const QImage& img);

protected:
	virtual void processCore();
};