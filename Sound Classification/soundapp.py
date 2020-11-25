
# coding: utf-8

# In[1]:


from flask import Flask
from flask import jsonify
from flask import request
import io
import os


# In[2]:


import keras
import tensorflow as tf
import librosa
import numpy as np


# In[3]:


from keras.utils.np_utils import to_categorical


# In[3]:


app = Flask(__name__)
new_model = None

# In[4]:


def load_model():
    global new_model
    global graph
    new_model = keras.models.load_model('my_urban_sound_model_addsiren.h5')
    new_model._make_predict_function()
    graph = tf.get_default_graph()
    print("Model loaded")
    
# In[ ]:


def prepare_wav(soundfile): 
    y,sr=librosa.load(soundfile)
    mfccs = np.mean(librosa.feature.mfcc(y, sr, n_mfcc=40).T,axis=0)
    melspectrogram = np.mean(librosa.feature.melspectrogram(y=y, sr=sr, n_mels=40,fmax=22050).T,axis=0)
    chroma_stft=np.mean(librosa.feature.chroma_stft(y=y, sr=sr,n_chroma=40).T,axis=0)
    chroma_cq = np.mean(librosa.feature.chroma_cqt(y=y, sr=sr,n_chroma=40).T,axis=0)
    chroma_cens = np.mean(librosa.feature.chroma_cens(y=y, sr=sr,n_chroma=40).T,axis=0)
    features=np.reshape(np.vstack((mfccs,melspectrogram,chroma_stft,chroma_cq,chroma_cens )),(40,5))
    features= np.reshape(features, (1, 40, 5))
    features= np.reshape(features, (1, 40, 5, 1))
    return features


# In[4]:

load_model()

@app.route('/predict', methods=['GET','POST'])
def predict():
    if request.method == "POST":
        if request.files.get("wave"):
            wavename = request.files["wave"].filename
            f = request.files["wave"]
            f.save(f.filename)
            features = prepare_wav(wavename) 
            with graph.as_default():   
                out_pred = new_model.predict(features)
                pred = np.argmax(out_pred)
                if pred == 0:
                    outstr = "air_conditioner"
                if pred == 1:
                    outstr = "car_horn"
                if pred == 2:
                    outstr = "children_playing"
                if pred == 3:
                    outstr = "dog_bark"
                if pred == 4:
                    outstr = "drilling"
                if pred == 5:
                    outstr = "engine_idling"
                if pred == 6:
                    outstr = "gun_shot"
                if pred == 7:
                    outstr = "jackhammer"
                if pred == 8:
                    outstr = "Siren"
                elif pred == 9:
                    outstr = "street_music"  
    return jsonify(outstr)
 
# In[ ]:


if __name__ == '__main__':
    #load_model()
    app.run(host='0.0.0.0', debug=True)
           

