#include <QDir>
#include <QList>



class ShaderEntry
{
public:
	ShaderEntry( QString aName, QString aShader )
		: name( aName ),
		shader( aShader )
	{}
	
	QString getName() const { return name; }
	QString getShader() const { return shader; }
	
	
private:
	QString name;
	QString shader;
};



class ShaderCollection
{
public:
	static ShaderCollection* getGlobalInstance();
	QStringList localShadersNames();
	QString getLocalShader( QString aName );
	bool localShaderExists( QString aName );
	bool saveLocalShader( QString aName, QString aShader );
	
private:
	ShaderCollection();	
	bool cdLocalShaderDir( QDir &dir );

	QList<ShaderEntry> localShaders;
	static ShaderCollection globalInstance;
};