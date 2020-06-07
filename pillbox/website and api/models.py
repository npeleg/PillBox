from server import login


@login.user_loader
def load_user(id):
	return id

	
