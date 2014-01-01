from distutils.core import setup
import py2exe

setup(
    windows = ['upng.py'],
    data_files = [
		('', [
			'upng.ui']
		),
		('imageformats', [
			'imageformats/qgif4.dll', 
			'imageformats/qsvg4.dll', 
			'imageformats/qtiff4.dll', 
			'imageformats/qjpeg4.dll'
			]
		)
	],
    options = {
        'py2exe': {
            'includes': ['PySide.QtXml'],
            'dist_dir': 'any2png',
        }
    }
)
