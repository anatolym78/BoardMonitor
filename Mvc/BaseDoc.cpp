#include "BaseDoc.h"

#include <iostream>
#include <exception>

BaseDoc::BaseDoc(QObject *parent) : QObject(parent)
{
}

BaseDoc::~BaseDoc()
{
}

bool BaseDoc::isValid()
{
	return !m_image.isNull();
}

void BaseDoc::clear()
{
	m_image = QImage();
}

bool BaseDoc::isImageLoaded()
{
	return !m_image.isNull();
}

bool BaseDoc::loadImage(const QString& fname)
{
	clear();

	if (!m_image.load(fname)) return false;


	//bool fuckingQImageBug = true;
	//if(!fuckingQImageBug)
	//{
	//	QImage img;
	//	if(!img.load(fname)) return false;

	//	clear();

	//	ConvImage::Convert(img, m_image);
	//}
	//else
	//{
	//	CImage img;
	//	if(img.Load(QtToStd(fname)) != 0) return false;
	//	img.Save("C:/Images/Iris/JpgTest.bmp");

	//	ConvImg::Convert(img, m_image);
	//}

	emit imageUpdated(m_image);

	return true;
}

void BaseDoc::setImage(const QImage& img)
{
    try
    {
        bool isNewSize = true;
        if(!m_image.isNull() && !img.isNull())
        {
            isNewSize = m_image.size() != img.size();
        }

        clear();

        m_image = img.copy();

        if(isNewSize)
        {
            emit imageUpdated(m_image);
        }
    }
    catch(std::exception ex)
    {
        std::cout<<ex.what()<<std::endl;
    }
}

int BaseDoc::imgWidth()
{
	if(isImageLoaded())
	{
		return m_image.width();
	}
	else
	{
		return 0;
	}
}

int BaseDoc::imgHeight()
{
	if(isImageLoaded())
	{
		return m_image.height();
	}
	else
	{
		return 0;
	}
}

void BaseDoc::process()
{
	processCore();
}

void BaseDoc::stopProcess()
{

}

void BaseDoc::processCore()
{

}

void BaseDoc::updateViews()
{
	emit dataUpdated();
}
