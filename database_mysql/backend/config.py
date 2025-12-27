# 数据库连接配置（替换为你的MySQL账号密码）
SQLALCHEMY_DATABASE_URI = "mysql+pymysql://root:20061030xcy@127.0.0.1:3306/student_score_system"
SQLALCHEMY_TRACK_MODIFICATIONS = False
SECRET_KEY = "your-secret-key"  # Flask会话密钥

# 新增：会话配置（用于存储登录状态）
SESSION_TYPE = 'filesystem'  # 会话存储在本地文件
SESSION_FILE_DIR = './flask_sessions'  # 会话文件存储目录
SESSION_PERMANENT = False  # 非永久会话（关闭浏览器失效）
PERMANENT_SESSION_LIFETIME = 3600  # 会话有效期1小时（秒）
SESSION_USE_SIGNER = True  # 会话ID签名，防止篡改