#include "imageprocessor.h"
#include <QThreadPool>
#include <QList>
#include <QFile>
#include <QFileInfo>
#include <QRunnable>
#include <QCoreApplication>
#include <QPointer>
#include <QUrl>
#include <QImage>
#include <QDir>
#include <QDebug>

typedef void (*AlgorithmFuntion)(QString souceFile,QString destFile);
class AlgorithmRunnable;

class ExcutedEvent:public QEvent
{
public:
    ExcutedEvent(AlgorithmRunnable *r):
        QEvent(evType()),mRunnable(r)
    {

    }

    static QEvent::Type evType(){
        if(sEvType==QEvent::None){
            sEvType=(QEvent::Type)registerEventType();
        }
        return sEvType;
    }

//data
public:
    AlgorithmRunnable *mRunnable;


private:
    static QEvent::Type sEvType;
};


QEvent::Type ExcutedEvent::sEvType=QEvent::None;

static void _gray(QString sourceFile,QString destFile);
static void _binarize(QString sourceFile,QString destFile);
static void _negative(QString sourceFile,QString destFile);
static void _emboss(QString sourceFile,QString destFile);
static void _sharpen(QString sourceFile,QString destFile);
static void _soften(QString sourceFile,QString destFile);


//全局存储的各个static 函数的指针
static AlgorithmFuntion g_functions[ImageProcessor::AlogrithmCount]={

    _gray,
    _binarize,
    _negative,
    _emboss,
    _sharpen,
    _soften
};

class AlgorithmRunnable:public QRunnable
{
public:
    AlgorithmRunnable(QString sourceFile,QString destFile,
                      ImageProcessor::ImageAlgorithm alg, QObject *observer) :

        Mobserver(observer),
        mSourceFilePath(sourceFile),
        mDestFilePath(destFile),
        mAlg(alg)
    {

    }

    void run(){
        g_functions[mAlg](mSourceFilePath,mDestFilePath);
        QCoreApplication::postEvent(Mobserver,new ExcutedEvent(this));
    }

public:
    QPointer<QObject> Mobserver;
    QString mSourceFilePath;
    QString mDestFilePath;
    ImageProcessor::ImageAlgorithm mAlg;
};


class ImageProcessorPrivate:public QObject
{
public:
    ImageProcessorPrivate(ImageProcessor *proc):
        QObject(proc),
        m_processor(proc),
        m_tempPath(QDir::currentPath())

    {

    }

    bool event(QEvent *event){
        if(event->type()==ExcutedEvent::evType()){
            ExcutedEvent *ee=(ExcutedEvent *)event;
            if(m_runnables.contains(ee->mRunnable)){
                m_notifiedAlgorithm=ee->mRunnable->mAlg;
                m_notifiedSourceFile=ee->mRunnable->mSourceFilePath;
                emit m_processor->finished(ee->mRunnable->mDestFilePath);
                m_runnables.removeOne(ee->mRunnable);
            }

            delete ee->mRunnable;
            return true;
        }

        return QObject::event(event);
    }

    void process(QString sourceFile,ImageProcessor::ImageAlgorithm alg)
    {
        QFileInfo fi(sourceFile);
        QString destFile=QString("$1/%2_%3").arg(m_tempPath).arg((int)alg).arg(fi.fileName());

        AlgorithmRunnable *r=new AlgorithmRunnable(sourceFile,destFile,alg,this);
        m_runnables.append(r);
        r->setAutoDelete(false);
        QThreadPool::globalInstance()->start(r);
    }

    ImageProcessor *m_processor;
    QList<AlgorithmRunnable*> m_runnables;
    QString m_notifiedSourceFile;
    ImageProcessor::ImageAlgorithm m_notifiedAlgorithm;
    QString m_tempPath;
};



ImageProcessor::ImageProcessor(QObject *parent) : QObject(parent),m_d(new ImageProcessorPrivate(this))
{

}

ImageProcessor::~ImageProcessor()
{
    delete m_d;
}


QString ImageProcessor::sourceFile()const
{
    return m_d->m_notifiedSourceFile;
}

ImageProcessor::ImageAlgorithm ImageProcessor::algorithm() const
{
    return m_d->m_tempPath;
}

void ImageProcessor::setTempPath(QString tempPath)
{
    m_d->m_tempPath=tempPath;
}

void ImageProcessor::process(QString file,ImageAlgorithm algorithm)
{
    m_d->process(file,algorithm);
}
void ImageProcessor::abort(QString file,ImageAlgorithm alg)
{
    int size=m_d->m_runnables.size();
    AlgorithmRunnable *r;
    for(int i=0;i<size;i++)
    {
        r=m_d->m_runnables.at(i);
        if(r->mSourceFilePath==file && r->mAlg==alg){
            m_d->m_runnables.removeAt(i);
            break;
        }
    }
}
