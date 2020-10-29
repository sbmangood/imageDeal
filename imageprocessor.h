#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QObject>
#include <QString>

class ImageProcessorPrivate;

class ImageProcessor : public QObject
{
    Q_OBJECT
    Q_ENUMS(ImageAlgorithm)
    Q_PROPERTY(QString sourceFile
               READ sourceFile )
    Q_PROPERTY(ImageAlgorithm algorithm
               READ algorithm )
public:
    explicit ImageProcessor(QObject *parent = nullptr);
    ~ImageProcessor();
    //增加branch

    //main change

    //增加branch2

    //测试分支


    enum ImageAlgorithm{
        Gray=0,                 //灰色
        Binarize,               //黑白
        Negative,               //底片
        Emboss,                 //浮雕
        Sharpen,                //锐化
        Soften,                 //柔光
        AlogrithmCount          //这个放在最后,可以自动计算出枚举类型的个数
    };


    QString sourceFile() const;
    ImageAlgorithm algorithm() const;
    void setTempPath(QString tempPath);
signals:
    void finished(QString newFile);
    void progress(int value);

public slots:
    void process(QString file,ImageAlgorithm algorithm);
    void abort(QString file,ImageAlgorithm algorithm);

private:
    ImageProcessorPrivate *m_d;
};

#endif // IMAGEPROCESSOR_H
