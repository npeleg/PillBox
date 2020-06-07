# Python Web Server
from utils import *
from cred import *
from flask import Flask, escape, request, render_template, redirect, url_for, make_response
from forms import SubscriberDelete, addSubscriberForm, LoginForm, AddTopicForm, TestTopicPublishForm
from config import Config
#from flask_login import LoginManager, current_user, login_user, UserMixin
import pathlib
from flask_restful import Resource, Api
from rest import *


app = Flask(__name__)
app.config.from_object(Config)
api = Api(app)
#login = LoginManager(app)

#class User(UserMixin):
#	def __init__(self, username):
#		self.username = username

api.add_resource(publishTopicFromBoard, '/api/autopublish')


@app.route('/logout')
def logout():
	response = make_response(redirect(url_for('login')))
	response.set_cookie('username', '', expires=0)
	return response


@app.route('/admin')
def admin():
	username = request.cookies.get('username')
	if username == None:
		return redirect(url_for('login'))
	if username != "admin":
		return redirect('/client/' + username)
	topics = getAllTopics()
	return render_template('all_clients.html', topics=topics)


@app.route('/register', methods=['GET', 'POST'])
def register():
	username = request.cookies.get('username')
	if username == None:
		return redirect(url_for('login'))
	if username != "admin":
		return redirect('/client/' + username)
	form = AddTopicForm()
	if form.validate_on_submit():
		result = addTopic(form.topic.data)
		print("---" + str(result) + "---")
		if result == 0:
			path = pathlib.Path('users/'+form.topic.data)
			path.touch()
			path.write_text(form.password.data)
			return redirect(url_for('admin'))
		return redirect(url_for('register'))		
	return render_template('register.html', form=form)


@app.route('/client/<name>/add', methods=['GET', 'POST'])
def client_add(name):
	username = request.cookies.get('username')
	if username == None:
		return redirect(url_for('login'))
	if (username != name) and (username != "admin"):
		return redirect('/client/' + username)
	addForm = addSubscriberForm()
	if addForm.validate_on_submit():
		print(request.form['phone_number'])
		addSubscriber(name, request.form['phone_number'])
		return redirect('/client/'+name)
	return render_template('client_add.html', topic=name, add=addForm)


@app.route('/client/<name>/testpublish', methods=['GET', 'POST'])
def test_publish(name):
	username = request.cookies.get('username')
	if username == None:
		return redirect(url_for('login'))
	if (username != name) and (username != "admin"):
		return redirect('/client/' + username)
	form = TestTopicPublishForm()
	
	if form.validate_on_submit():
		print("Publish Test for client " + str(name) + " : " + str(form.message.data))
		publishTopic(name, form.message.data, form.sender.data)
		return redirect('/client/'+name)
	return render_template('test_publish.html', form=form, topic=name)


@app.route('/client/<name>', methods=['GET', 'POST'])
def client_display(name):
	username = request.cookies.get('username')
	if username == None:
		return redirect(url_for('login'))
	if (username != name) and (username != "admin"):
		return redirect('/client/' + username)
	subs = getTopicSubscribers(name)
	subs_canonized = [deCanonize(sub) for sub in subs]
	form = SubscriberDelete()

	if form.validate_on_submit():
		print("")
		for subscriber in subs_canonized:
			if subscriber in request.form:
				removeSubscriber(name, subscriber)
		return redirect('/client/'+name)

	return render_template('display_client.html', topic=name, subscribers=subs_canonized, form=form)

@app.route('/', methods=['GET', 'POST'])
@app.route('/login', methods=['GET', 'POST'])
def login():
	username = request.cookies.get('username')
	if username != None:
		return redirect('/client/' + username)
	form = LoginForm()
	if form.validate_on_submit():
		path = pathlib.Path("users/"+form.topic.data)
		if not path.is_file():
			return redirect(url_for('login'))
		with open("users/"+form.topic.data) as f:
			content = f.readline().rstrip('\n')
		print(content)
		print(form.password.data)
		if form.password.data == content: # user connected
			if(form.topic.data == "admin"):
				response = make_response(redirect(url_for('admin')))
				response.set_cookie('username', 'admin')
				return response
			else:
				response = make_response(redirect('/client/'+form.topic.data))
				response.set_cookie('username', form.topic.data)
				return response
	return render_template('login.html', form=form)

if __name__ == '__main__':
	app.run()
