from flask_wtf import FlaskForm
from wtforms import StringField, PasswordField, BooleanField, SubmitField
from wtforms.validators import DataRequired

class SubscriberDelete(FlaskForm):
	delete = SubmitField('Delete')


class addSubscriberForm(FlaskForm):
	phone_number = StringField('Phone Number:', validators=[DataRequired()])
	submit = SubmitField('Add')

class LoginForm(FlaskForm):
	topic = StringField('Id', validators=[DataRequired()])
	password = StringField('Password', validators=[DataRequired()])
	submit = SubmitField('Sign in')

class AddTopicForm(FlaskForm):
	topic = StringField('New Client Name:', validators=[DataRequired()])
	password = StringField('Password:', validators=[DataRequired()])
	submit = SubmitField('Register')


class TestTopicPublishForm(FlaskForm):
	message = StringField('Message to send:', validators=[DataRequired()])
	sender = StringField('Sender ID: (can leave blank')
	submit = SubmitField('Send')
