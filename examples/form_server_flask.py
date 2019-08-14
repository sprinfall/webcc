# A demo HTTP file upload server modified from:
#   http://flask.pocoo.org/docs/1.0/patterns/fileuploads/#uploading-files
# Run:
# (Windows)
# $ set FLASK_APP=form_server_flask.py
# $ flask 

import os
from flask import Flask, flash, request, redirect, url_for
from flask import send_from_directory
from werkzeug.utils import secure_filename


UPLOAD_FOLDER = '/path/to/the/uploads'
app = Flask(__name__)
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER


@app.route('/upload', methods=['POST'])
def upload_file():
    if request.method == 'POST':
        for name, data in request.form.items():
            print(f'form: {name}, {data}')

        for name, file in request.files.items():
            if file:
                secured_filename = secure_filename(file.filename)
                print(f"file:  {name}, {file.filename} ({secured_filename})")
                file.save(os.path.join(app.config['UPLOAD_FOLDER'],
                    secured_filename))

        return "OK"


@app.route('/uploads/<filename>')
def uploaded_file(filename):
    return send_from_directory(app.config['UPLOAD_FOLDER'], filename)
